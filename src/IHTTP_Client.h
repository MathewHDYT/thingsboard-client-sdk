#ifndef IHTTP_Client_h
#define IHTTP_Client_h

// Local include.
#include "Configuration.h"

// Library include.
#if THINGSBOARD_ENABLE_STL
#include <string>
#else
#include <WString.h>
#endif // THINGSBOARD_ENABLE_STL


/// @brief HTTP Client interface that contains the method that a class that can be used to send and receive data over an HTTP conection should implement
/// @note Seperates the specific implementation used from the ThingsBoardHttp client, allows to use different clients depending on different needs.
/// In this case the main use case of the seperation is to both support Espressif IDF and Arduino with the following libraries as recommendations.
/// The default HTTP Client for Arduino is the ArduinoHttpClient (https://github.com/arduino-libraries/ArduinoHttpClient),
/// For Espressif IDF however the default HTTP Client is the esp-http-client (https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/protocols/esp_http_client.html) component.
/// The aforementioned recommendations are already partially implemented in the library and can can simply be used and included when using the library, for Arduino the Arduino_HTTP_Client can simply be included
/// and for Espressif IDF the implementations has not been created yet, the implementations have been tested and should be compatible when used in conjunction with the ThingsBoardHttp client
class IHTTP_Client {
  public:
    /// @copydoc Callback::~Callback
    virtual ~IHTTP_Client() {}

    /// @brief Sets whether to close the HTTP connection for every single request and reconnect once a new request is sent
    /// @param keep_alive Enable or disable to keep the connection alive after a message has been sent
    /// Is recommended to be always enabled to improve performance and speed, because opening a new connection takes a while especially when using HTTPS
    virtual void set_keep_alive(bool keep_alive) = 0;

    /// @brief Connects to the given server instance over the defined port
    /// @param host Non owning pointer to server instance name the client should connect too.
    /// Does not need to be kept alive, because the server name is only used for the scope of the method itself
    /// @param port Port that will be used to establish a connection and send / receive data
    /// Should be either 80 for unencrypted HTTP or 443 for HTTPS with encryption.
    /// The latter is recommended if relevant data is sent or if the client receives and handles requests from the server,
    /// because using an unencrpyted connection, will allow 3rd parties to listen to the communication and impersonate the server sending payloads which might influence the device in unexpected ways
    /// @return Whether the client could establish the connection successfully with return code 0 or failed with error code otherwise
    virtual int connect(char const * host, uint16_t port) = 0;

    /// @brief Disconnects the given device from the current host and clears about any remaining bytes still in the reponse body
    virtual void stop() = 0;

    /// @brief Connects to the server and sends a POST request with a body and content type
    /// @param url_path Non owning pointer to URL path the POST request should be sent too.
    /// Does not need to be kept alive, because the url path is only used for the scope of the method itself
    /// @param content_type Non owning pointer to type of the content that is sent will be JSON data most of the time.
    /// Does not need to be kept alive, because the content type is only used for the scope of the method itself
    /// @param request_body Non owning pointer to request body containing data of the given content type.
    /// Does not need to be kept alive, because the request body is only used for the scope of the method itself
    /// @return Whether the request was successful or not, returns 0 if successful or if not the internal error code
    virtual int post(char const * url_path, char const * content_type, char const * request_body) = 0;

    /// @brief Gets the HTTP status code contained in the server response.
    /// @note Should follow the HTTP standard, meaning 200 for a successful request or 404 for file not found,
    /// see https://developer.mozilla.org/en-US/docs/Web/HTTP/Status for more information on all standard status codes
    /// @return Current HTTP status code from the server response
    virtual int get_response_status_code() = 0;

    /// @brief Connects to the server and sends a GET request
    /// @param url_path Non owning pointer to URL path the GET request should be sent too.
    /// Does not need to be kept alive, because the url path is only used for the scope of the method itself
    /// @return Whether the request was successful or not, returns 0 if successful or if not the internal error code
    virtual int get(const char *url_path) = 0;

    /// @brief Returns the response body of a previously sent message as a string object,
    /// skips any response headers if they have not been read already,
    /// should be called after calling get_response_status_code() and ensuring the request was successful
    /// @return Response body of a request
#if THINGSBOARD_ENABLE_STL
    virtual std::string get_response_body() = 0;
#else
    virtual String get_response_body() = 0;
#endif // THINGSBOARD_ENABLE_STL
};

#endif // IHTTP_Client_h
