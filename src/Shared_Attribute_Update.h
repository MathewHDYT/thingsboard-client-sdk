#ifndef Shared_Attribute_Update_h
#define Shared_Attribute_Update_h

// Local includes.
#include "Shared_Attribute_Callback.h"
#include "API_Implementation.h"


// Log messages.
#if THINGSBOARD_ENABLE_DEBUG
char constexpr NOT_FOUND_ATT_UPDATE[] = "Shared attribute update key not found";
char constexpr ATT_CB_NO_KEYS[] = "No keys subscribed. Calling subscribed callback for any updated attributes, assumed to be subscribed to every possible key";
char constexpr ATT_NO_CHANGE[] = "No keys that we subscribed too were changed, skipping callback";
char constexpr SHARED_KEY_IS_NULL[] = "Subscribed shared attribute update key is NULL";
char constexpr CALLING_ATT_CB[] = "Calling subscribed callback for updated shared attribute (%s)";
#endif // THINGSBOARD_ENABLE_DEBUG
#if !THINGSBOARD_ENABLE_DYNAMIC
char constexpr SHARED_ATTRIBUTE_UPDATE_SUBSCRIPTIONS[] = "shared attribute update";
#endif // !THINGSBOARD_ENABLE_DYNAMIC


/// @brief Handles the internal implementation of the ThingsBoard shared attribute update API.
/// See https://thingsboard.io/docs/reference/mqtt-api/#subscribe-to-attribute-updates-from-the-server for more information
/// @tparam Logger Implementation that should be used to print error messages generated by internal processes and additional debugging messages if THINGSBOARD_ENABLE_DEBUG is set, default = DefaultLogger
#if THINGSBOARD_ENABLE_DYNAMIC
template <typename Logger = DefaultLogger>
#else
/// @tparam MaxSubscribtions Maximum amount of simultaneous server side rpc subscriptions.
/// Once the maximum amount has been reached it is not possible to increase the size, this is done because it allows to allcoate the memory on the stack instead of the heap, default = Default_Subscriptions_Amount (2)
/// @tparam MaxAttributes Maximum amount of attributes that will ever be requested with the Shared_Attribute_Callback, allows to use an array on the stack in the background, default = Default_Attributes_Amount (5)
template<size_t MaxSubscribtions = Default_Subscriptions_Amount, size_t MaxAttributes = Default_Attributes_Amount, typename Logger = DefaultLogger>
#endif // THINGSBOARD_ENABLE_DYNAMIC
class Shared_Attribute_Update : public API_Implementation {
  public:
    /// @brief Constructor
    Shared_Attribute_Update() = default;

    /// @brief Subscribes multiple shared attribute callbacks,
    /// that will be called if the key-value pair from the server for the given shared attributes is received.
    /// See https://thingsboard.io/docs/reference/mqtt-api/#subscribe-to-attribute-updates-from-the-server for more information
    /// @tparam InputIterator Class that points to the begin and end iterator
    /// of the given data container, allows for using / passing either std::vector or std::array.
    /// See https://en.cppreference.com/w/cpp/iterator/input_iterator for more information on the requirements of the iterator
    /// @param first Iterator pointing to the first element in the data container
    /// @param last Iterator pointing to the end of the data container (last element + 1)
    /// @return Whether subscribing the given callbacks was successful or not
    template<typename InputIterator>
    bool Shared_Attributes_Subscribe(InputIterator const & first, InputIterator const & last) {
#if !THINGSBOARD_ENABLE_DYNAMIC
        size_t const size = Helper::distance(first, last);
        if (m_shared_attribute_update_callbacks.size() + size > m_shared_attribute_update_callbacks.capacity()) {
            Logger::printfln(MAX_SUBSCRIPTIONS_EXCEEDED, SHARED_ATTRIBUTE_UPDATE_SUBSCRIPTIONS);
            return false;
        }
#endif // !THINGSBOARD_ENABLE_DYNAMIC
        if (!m_subscribe_callback.Call_Callback(ATTRIBUTE_TOPIC)) {
            Logger::printfln(SUBSCRIBE_TOPIC_FAILED, ATTRIBUTE_TOPIC);
            return false;
        }

        // Push back complete vector into our local m_shared_attribute_update_callbacks vector.
        m_shared_attribute_update_callbacks.insert(m_shared_attribute_update_callbacks.end(), first, last);
        return true;
    }

    /// @brief Subscribe one shared attribute callback,
    /// that will be called if the key-value pair from the server for the given shared attributes is received.
    /// See https://thingsboard.io/docs/reference/mqtt-api/#subscribe-to-attribute-updates-from-the-server for more information
    /// @param callback Callback method that will be called
    /// @return Whether subscribing the given callback was successful or not
#if THINGSBOARD_ENABLE_DYNAMIC
    bool Shared_Attributes_Subscribe(Shared_Attribute_Callback const & callback) {
#else
    bool Shared_Attributes_Subscribe(Shared_Attribute_Callback<MaxAttributes> const & callback) {
#endif // THINGSBOARD_ENABLE_DYNAMIC
#if !THINGSBOARD_ENABLE_DYNAMIC
        if (m_shared_attribute_update_callbacks.size() + 1U > m_shared_attribute_update_callbacks.capacity()) {
            Logger::printfln(MAX_SUBSCRIPTIONS_EXCEEDED, SHARED_ATTRIBUTE_UPDATE_SUBSCRIPTIONS);
            return false;
        }
#endif // !THINGSBOARD_ENABLE_DYNAMIC
        if (!m_subscribe_callback.Call_Callback(ATTRIBUTE_TOPIC)) {
            Logger::printfln(SUBSCRIBE_TOPIC_FAILED, ATTRIBUTE_TOPIC);
            return false;
        }

        // Push back given callback into our local vector
        m_shared_attribute_update_callbacks.push_back(callback);
        return true;
    }

    /// @brief Unsubcribes all shared attribute callbacks.
    /// See https://thingsboard.io/docs/reference/mqtt-api/#subscribe-to-attribute-updates-from-the-server for more information
    /// @return Whether unsubcribing all the previously subscribed callbacks
    /// and from the attribute topic, was successful or not
    bool Shared_Attributes_Unsubscribe() {
        m_shared_attribute_update_callbacks.clear();
        return m_unsubscribe_callback.Call_Callback(ATTRIBUTE_TOPIC);
    }

    char const * Get_Response_Topic_String() const override {
        return ATTRIBUTE_TOPIC;
    }

    bool Unsubscribe() override {
        return Shared_Attributes_Unsubscribe();
    }

    bool Resubscribe_Topic() override {
        if (!m_shared_attribute_update_callbacks.empty() && !m_subscribe_callback.Call_Callback(ATTRIBUTE_TOPIC)) {
            Logger::printfln(SUBSCRIBE_TOPIC_FAILED, ATTRIBUTE_TOPIC);
        }
        return true;
    }

    void Process_Json_Response(char * const topic, JsonObjectConst & data) override {
        if (!data) {
#if THINGSBOARD_ENABLE_DEBUG
            Logger::println(NOT_FOUND_ATT_UPDATE);
#endif // THINGSBOARD_ENABLE_DEBUG
            return;
        }

        if (data.containsKey(SHARED_RESPONSE_KEY)) {
            data = data[SHARED_RESPONSE_KEY];
        }

        for (auto const & shared_attribute : m_shared_attribute_update_callbacks) {
            if (shared_attribute.Get_Attributes().empty()) {
#if THINGSBOARD_ENABLE_DEBUG
                Logger::println(ATT_CB_NO_KEYS);
#endif // THINGSBOARD_ENABLE_DEBUG
                // No specifc keys were subscribed so we call the callback anyway, assumed to be subscribed to any update
                shared_attribute.Call_Callback(data);
                continue;
            }

            char const * requested_att = nullptr;

            for (auto const & att : shared_attribute.Get_Attributes()) {
                if (Helper::stringIsNullorEmpty(att)) {
#if THINGSBOARD_ENABLE_DEBUG
                    Logger::println(SHARED_KEY_IS_NULL);
#endif // THINGSBOARD_ENABLE_DEBUG
                    continue;
                }
                // Check if the request contained any of our requested keys and
                // break early if the key was requested from this callback.
                if (data.containsKey(att)) {
                    requested_att = att;
                    break;
                }
            }

            // Check if this callback did not request any keys that were in this response,
            // if there were not we simply continue with the next subscribed callback.
            if (requested_att == nullptr) {
#if THINGSBOARD_ENABLE_DEBUG
                Logger::println(ATT_NO_CHANGE);
#endif // THINGSBOARD_ENABLE_DEBUG
                continue;
            }

#if THINGSBOARD_ENABLE_DEBUG
            Logger::printfln(CALLING_ATT_CB, requested_att);
#endif // THINGSBOARD_ENABLE_DEBUG
            shared_attribute.Call_Callback(data);
        }
    }

  private:
    // Vectors or array (depends on wheter if THINGSBOARD_ENABLE_DYNAMIC is set to 1 or 0), hold copy of the actual passed data, this is to ensure they stay valid,
    // even if the user only temporarily created the object before the method was called.
    // This can be done because all Callback methods mostly consists of pointers to actual object so copying them
    // does not require a huge memory overhead and is acceptable especially in comparsion to possible problems that could
    // arise if references were used and the end user does not take care to ensure the Callbacks live on for the entirety
    // of its usage, which will lead to dangling references and undefined behaviour.
    // Therefore copy-by-value has been choosen as for this specific use case it is more advantageous,
    // especially because at most we copy internal vectors or array, that will only ever contain a few pointers
#if THINGSBOARD_ENABLE_DYNAMIC
    Vector<Shared_Attribute_Callback>                                 m_shared_attribute_update_callbacks; // Shared attribute update callbacks vector
#else
    Array<Shared_Attribute_Callback<MaxAttributes>, MaxSubscribtions> m_shared_attribute_update_callbacks; // Shared attribute update callbacks array
#endif // THINGSBOARD_ENABLE_DYNAMIC
};

#endif // Shared_Attribute_Update_h
