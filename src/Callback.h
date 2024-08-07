#ifndef Callback_h
#define Callback_h

// Local includes.
#include "Configuration.h"
#if !THINGSBOARD_ENABLE_STL && THINGSBOARD_ENABLE_DYNAMIC
#include "Vector.h"
#else
#include "Array.h"
#endif // !THINGSBOARD_ENABLE_STL && THINGSBOARD_ENABLE_DYNAMIC

// Library includes.
#include <ArduinoJson.h>
#if THINGSBOARD_ENABLE_STL
#include <functional>
#include <vector>
#endif // THINGSBOARD_ENABLE_STL


#if THINGSBOARD_ENABLE_STL && THINGSBOARD_ENABLE_DYNAMIC
/// @brief Vector signature, makes it possible to use the Vector name everywhere instead of having to differentiate between C++ STL support or not
template<typename T>
using Vector = std::vector<T>;
#endif // THINGSBOARD_ENABLE_STL && THINGSBOARD_ENABLE_DYNAMIC


/// @brief General purpose callback wrapper. Expects either c-style or c++ style function pointer,
/// depending on if the C++ STL has been implemented on the given device or not
/// @tparam returnType Type the given callback method should return
/// @tparam argumentTypes Types the given callback method should receive
template<typename returnType, typename... argumentTypes>
class Callback {
  public:
    /// @brief Callback signature
#if THINGSBOARD_ENABLE_STL
    using function = std::function<returnType(argumentTypes... arguments)>;
#else
    using function = returnType (*)(argumentTypes... arguments);
#endif // THINGSBOARD_ENABLE_STL

    /// @brief Constructs empty callback, will result in never being called. Internals are simply default constructed as nullptr
    Callback() = default;

    /// @brief Constructs base callback, will be called upon specific arrival of json message
    /// where the requested data was sent by the cloud and received by the client
    /// @param callback Callback method that will be called upon data arrival with the given data that was received serialized into the given arguemnt types
    /// @param message Message that is logged if the callback given initally is a nullptr and can therefore not be called,
    /// used to ensure users are informed that the initalization of the child class is invalid
    explicit Callback(function callback)
      : m_callback(callback)
    {
        // Nothing to do
    }

    /// @brief Calls the callback that was subscribed, when this class instance was initally created
    /// @param ...arguments Received client-side or shared attribute request data that include
    /// the client-side or shared attributes that were requested and their current values
    returnType Call_Callback(argumentTypes const &... arguments) const {
        if (!m_callback) {
          return returnType();
        }
        return m_callback(arguments...);
    }

    /// @brief Sets the callback method that will be called upon data arrival with the given data that was received serialized into the given argument types,
    /// used to change the callback initally passed or to set the callback if it was not passed as an argument initally
    /// @param callback Callback method that will be called upon data arrival with the given data that was received serialized into the given argument types
    void Set_Callback(function callback) {
        m_callback = callback;
    }

  private:
    function   m_callback; // Callback to call
};

#endif // Callback_h
