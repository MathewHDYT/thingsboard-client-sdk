#ifndef RPC_Request_Callback_h
#define RPC_Request_Callback_h

// Local includes.
#include "Callback.h"


/// @brief Client-side RPC callback wrapper,
/// contains the needed configuration settings to create the request that should be sent to the server.
/// Documentation about the specific use of client-side RPC in ThingsBoard can be found here https://thingsboard.io/docs/user-guide/rpc/#client-side-rpc
class RPC_Request_Callback : public Callback<void, JsonVariantConst const &> {
  public:
    /// @brief Constructs empty callback, will result in never being called. Internals are simply default constructed as nullptr
    RPC_Request_Callback() = default;

    /// @brief Constructs callback, will be called upon client-side RPC response arrival originating
    /// from the original client side RPC request without any parameters
    /// @param methodName Name of the client side RPC method we want to call on the cloud
    /// @param callback Callback method that will be called upon data arrival with the given data that was received serialized into a JsonDocument
    RPC_Request_Callback(char const * const methodName, function callback);

    /// @brief Constructs callback, will be called upon RPC response arrival originating
    /// from the original client side RPC request with additional parameters that should be passed to the method
    /// @param methodName Name of the client side RPC method we want to call on the cloud
    /// @param parameteres Parameters that will be passed to the client side RPC,
    /// Optional, pass NULL if there are no argument for the given method
    /// @param callback Callback method that will be called upon data arrival with the given data that was received serialized into a JsonDocument
    RPC_Request_Callback(char const * const methodName, JsonArray const * const parameteres, function callback);

    /// @brief Gets the unique request identifier that is connected to the original request,
    /// and will be later used to verifiy which RPC_Request_Callback
    /// is connected to which received client-side RPC response
    /// @return Unique identifier connected to the request for client side rpc
    size_t const & Get_Request_ID() const;

    /// @brief Sets the unique request identifier that is connected to the original request,
    /// and will be later used to verifiy which RPC_Request_Callback
    /// is connected to which received client-side RPC response
    /// @param request_id Unique identifier connected to the request for client side rpc
    void Set_Request_ID(size_t const & request_id);

    /// @brief Gets the poiner to the underlying name of the client side RPC method we want to call on the cloud
    /// @return Pointer to the passed methodName
    char const * Get_Name() const;

    /// @brief Sets the poiner to the underlying name of the client side RPC method we want to call on the cloud
    /// @param methodName Pointer to the passed methodName
    void Set_Name(char const * const methodName);

    /// @brief Gets the pointer to the underlying paramaters we want to call the client side RPC method on the cloud with
    /// @return Pointer to the passed parameters
    JsonArray const * Get_Parameters() const;

    /// @brief Sets the pointer to the underlying paramaters we want to call the client side RPC method on the cloud with
    /// @param parameteres Pointer to the passed parameters
    void Set_Parameters(JsonArray const * const parameteres);

  private:
    char const        *m_methodName;  // Method name
    JsonArray const   *m_parameters;  // Parameter json
    size_t            m_request_id;   // Id the request was called with
};

#endif // RPC_Request_Callback_h
