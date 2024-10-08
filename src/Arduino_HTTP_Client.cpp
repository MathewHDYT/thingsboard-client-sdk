// Header include.
#include "Arduino_HTTP_Client.h"

#ifdef ARDUINO

Arduino_HTTP_Client::Arduino_HTTP_Client(Client& transport_client, char const * host, uint16_t port) :
    m_http_client(transport_client, host, port)
{
    // Nothing to do
}

void Arduino_HTTP_Client::set_keep_alive(bool keep_alive) {
    if (keep_alive) {
        m_http_client.connectionKeepAlive();
    }
}

int Arduino_HTTP_Client::connect(char const * host, uint16_t port) {
    return m_http_client.connect(host, port);
}

void Arduino_HTTP_Client::stop() {
    m_http_client.stop();
}

int Arduino_HTTP_Client::post(char const * url_path, char const * content_type, char const * request_body) {
    return m_http_client.post(url_path, content_type, request_body);
}

int Arduino_HTTP_Client::get_response_status_code() {
    return m_http_client.responseStatusCode();
}

int Arduino_HTTP_Client::get(char const * url_path) {
    return m_http_client.get(url_path);
}

#if THINGSBOARD_ENABLE_STL
std::string Arduino_HTTP_Client::get_response_body() {
    return m_http_client.responseBody().c_str();
#else
String Arduino_HTTP_Client::get_response_body() {
    return m_http_client.responseBody();
#endif // THINGSBOARD_ENABLE_STL
}

#endif // ARDUINO
