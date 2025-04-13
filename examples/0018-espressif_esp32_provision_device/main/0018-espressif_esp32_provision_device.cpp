#include <esp_netif.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <nvs_flash.h>
#include <string.h>

// Whether the given script is using encryption or not,
// generally recommended as it increases security (communication with the server is not in clear text anymore),
// it does come with an overhead tough as having an encrypted session requires a lot of memory,
// which might not be avaialable on lower end devices.
#define ENCRYPTED false

#include <Espressif_MQTT_Client.h>
#include <Provision.h>
#include <ThingsBoard.h>


// Whether the given script is using encryption or not,
// generally recommended as it increases security (communication with the server is not in clear text anymore),
// it does come with an overhead tough as having an encrypted session requires a lot of memory,
// which might not be avaialable on lower end devices.
#define ENCRYPTED false
// Whether the given script generates the deviceName from the included mac address on the WiFi chip,
// as a fallback option, if no other deviceName is given.
// If not a random string generated by the cloud will be used as the device name instead
#define USE_MAC_FALLBACK false


constexpr char WIFI_SSID[] = "YOUR_WIFI_SSID";
constexpr char WIFI_PASSWORD[] = "YOUR_WIFI_PASSWORD";

// Thingsboard we want to establish a connection too
constexpr char THINGSBOARD_SERVER[] = "demo.thingsboard.io";

// MQTT port used to communicate with the server, 1883 is the default unencrypted MQTT port,
// whereas 8883 would be the default encrypted SSL MQTT port
#if ENCRYPTED
constexpr uint16_t THINGSBOARD_PORT = 8883U;
#else
constexpr uint16_t THINGSBOARD_PORT = 1883U;
#endif

// Maximum size packets will ever be sent or received by the underlying MQTT client,
// if the size is to small messages might not be sent or received messages will be discarded
constexpr uint16_t MAX_MESSAGE_SEND_SIZE = 256U;
constexpr uint16_t MAX_MESSAGE_RECEIVE_SIZE = 256U;

#if ENCRYPTED
// See https://comodosslstore.com/resources/what-is-a-root-ca-certificate-and-how-do-i-download-it/
// on how to get the root certificate of the server we want to communicate with,
// this is needed to establish a secure connection and changes depending on the website.
constexpr char ROOT_CERT[] = R"(-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)";
#endif

// See https://thingsboard.io/docs/user-guide/device-provisioning/
// to understand how to create a device profile to be able to provision a device
constexpr char PROVISION_DEVICE_KEY[] = "YOUR_PROVISION_DEVICE_KEY";
constexpr char PROVISION_DEVICE_SECRET[] = "YOUR_PROVISION_DEVICE_SECRET";

// Optionally keep the device name empty and the WiFi mac address of the integrated
// wifi chip on ESP32 or ESP8266 will be used as the name instead
// Ensuring your device name is unique, even when reusing this code for multiple devices
constexpr char DEVICE_NAME[] = "";

constexpr char CREDENTIALS_TYPE[] = "credentialsType";
constexpr char CREDENTIALS_VALUE[] = "credentialsValue";
constexpr char CLIENT_ID[] = "clientId";
constexpr char CLIENT_PASSWORD[] = "password";
constexpr char CLIENT_USERNAME[] = "userName";
constexpr char TEMPERATURE_KEY[] = "temperature";
constexpr char HUMIDITY_KEY[] = "humidity";
constexpr char ACCESS_TOKEN_CRED_TYPE[] = "ACCESS_TOKEN";
constexpr char MQTT_BASIC_CRED_TYPE[] = "MQTT_BASIC";
constexpr char X509_CERTIFICATE_CRED_TYPE[] = "X509_CERTIFICATE";
constexpr char PROVISION_DEVICE_TASK_NAME[] = "provision_device_task";
constexpr uint64_t REQUEST_TIMEOUT_MICROSECONDS = 5000U * 1000U;

// Initalize the Mqtt client instance
Espressif_MQTT_Client<> mqttClient;
// Initialize used apis
Provision<> prov;
const std::array<IAPI_Implementation*, 1U> apis = {
    &prov
};
// Initialize ThingsBoard instance with the maximum needed buffer size
ThingsBoard tb(mqttClient, MAX_MESSAGE_RECEIVE_SIZE, MAX_MESSAGE_SEND_SIZE, DEFAULT_MAX_STACK_SIZE, apis);

uint32_t previous_processing_time = 0U;

// Status for successfully connecting to the given WiFi
bool wifi_connected = false;
// Statuses for provisioning
bool provisionRequestSent = false;
bool provisionResponseProcessed = false;

// Struct for client connecting after provisioning
struct Credentials {
  std::string client_id;
  std::string username;
  std::string password;
};
Credentials credentials;


/// @brief Callback method that is called if we got an ip address from the connected WiFi meaning we successfully established a connection
/// @param event_handler_arg User data registered to the event
/// @param event_base Event base for the handler
/// @param event_id The id for the received event
/// @param event_data The data for the event, esp_event_handler_t
void on_got_ip(void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    wifi_connected = true;
}

/// @brief Initalizes WiFi connection,
// will endlessly delay until a connection has been successfully established
void InitWiFi() {
    const wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));

    esp_netif_config_t netif_config = ESP_NETIF_DEFAULT_WIFI_STA();
    esp_netif_t *netif = esp_netif_new(&netif_config);
    assert(netif);

    ESP_ERROR_CHECK(esp_netif_attach_wifi_station(netif));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ip_event_t::IP_EVENT_STA_GOT_IP, &on_got_ip, NULL));
    ESP_ERROR_CHECK(esp_wifi_set_default_wifi_sta_handlers());
    ESP_ERROR_CHECK(esp_wifi_set_storage(wifi_storage_t::WIFI_STORAGE_RAM));

    wifi_config_t wifi_config;
    memset(&wifi_config, 0, sizeof(wifi_config));
    strncpy(reinterpret_cast<char*>(wifi_config.sta.ssid), WIFI_SSID, strlen(WIFI_SSID) + 1);
    strncpy(reinterpret_cast<char*>(wifi_config.sta.password), WIFI_PASSWORD, strlen(WIFI_PASSWORD) + 1);

    ESP_LOGI("MAIN", "Connecting to %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_set_mode(wifi_mode_t::WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(wifi_interface_t::WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_connect());
}

/// @brief Attribute request did not receive a response in the expected amount of microseconds 
void requestTimedOut() {
    ESP_LOGE("MAIN", "Provision request timed out did not receive a response in (%llu) microseconds. Ensure client is connected to the MQTT broker", REQUEST_TIMEOUT_MICROSECONDS);
}

/// @brief Process the provisioning response received from the server
/// @param data Reference to the object containing the provisioning response
void processProvisionResponse(const JsonDocument &data) {
    const size_t jsonSize = Helper::Measure_Json(data);
    char buffer[jsonSize];
    serializeJson(data, buffer, jsonSize);
    ESP_LOGI("MAIN", "Received device provision response: %s", buffer);

    if (strcmp(data["status"], "SUCCESS") != 0) {
        ESP_LOGE("MAIN", "Provision response contains the error: %s", data["errorMsg"].as<const char*>());
        return;
    }

    // Process credentials based on the type
    if (strncmp(data[CREDENTIALS_TYPE], ACCESS_TOKEN_CRED_TYPE, strlen(ACCESS_TOKEN_CRED_TYPE)) == 0) {
        credentials.client_id = "";
        credentials.username = data[CREDENTIALS_VALUE].as<std::string>();
        credentials.password = "";
    }
    else if (strncmp(data[CREDENTIALS_TYPE], MQTT_BASIC_CRED_TYPE, strlen(MQTT_BASIC_CRED_TYPE)) == 0) {
        auto credentials_value = data[CREDENTIALS_VALUE].as<JsonObjectConst>();
        credentials.client_id = credentials_value[CLIENT_ID].as<std::string>();
        credentials.username = credentials_value[CLIENT_USERNAME].as<std::string>();
        credentials.password = credentials_value[CLIENT_PASSWORD].as<std::string>();
    }
    else {
        ESP_LOGE("MAIN", "Unexpected provision credentialsType: %s", data[CREDENTIALS_VALUE].as<const char*>());
        return;
    }

    // Disconnect from the cloud client connected to the provision account, because it is no longer needed the device has been provisioned
    // and we can reconnect to the cloud with the newly generated credentials.
    if (tb.connected()) {
      tb.disconnect();
    }
    provisionResponseProcessed = true;
}

/// @brief Attempts to connect as a provision client, provision a device and then reconnect as that provisioned device
/// @param pvParameter Always null
void provision_device(void *pvParameters) {
    // Send a claiming request without any device name (random string will be used as the device name)
    // if the string is empty or null, automatically checked by the sendProvisionRequest method
    std::string device_name = DEVICE_NAME;

#if USE_MAC_FALLBACK
    // Check if passed DEVICE_NAME was empty,
    // and if it was get the mac address of the wifi chip as fallback and use that one instead
    if ((DEVICE_NAME == nullptr) || (DEVICE_NAME[0] == '\0')) {
        uint8_t mac[6];
        esp_wifi_get_mac(WIFI_IF_STA, mac);
        char mac_str[18];
        snprintf(mac_str, 18U, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        device_name = mac_str;
    }
#endif

    // Connect to the ThingsBoard server as a client wanting to provision a new device
    if (!tb.connect(THINGSBOARD_SERVER, "provision", THINGSBOARD_PORT)) {
        ESP_LOGE("MAIN", "Failed to connect to ThingsBoard server with provision account");
        return;
    }

    while (!tb.connected()) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    // Prepare and send the provision request
    const Provision_Callback provisionCallback(Access_Token(), &processProvisionResponse, PROVISION_DEVICE_KEY, PROVISION_DEVICE_SECRET, device_name.c_str(), REQUEST_TIMEOUT_MICROSECONDS, &requestTimedOut);
    provisionRequestSent = prov.Provision_Request(provisionCallback);

    // Wait for the provisioning response to be processed
    while (!provisionResponseProcessed) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    // Disconnect from the cloud client connected to the provision account
    // because the device has been provisioned and can reconnect with new credentials
    if (tb.connected()) {
        tb.disconnect();
    }

    // Wait for the provisioning device to disconnect
    while (tb.connected()) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    // Connect to the ThingsBoard server, as the provisioned client
    tb.connect(THINGSBOARD_SERVER, credentials.username.c_str(), THINGSBOARD_PORT, credentials.client_id.c_str(), credentials.password.c_str());

    tb.loop();
    vTaskDelete(NULL);
}

extern "C" void app_main(void) {
    ESP_LOGI("MAIN", "[APP] Startup..");
    ESP_LOGI("MAIN", "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
    ESP_LOGI("MAIN", "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    InitWiFi();

#if ENCRYPTED
    mqttClient.set_server_certificate(ROOT_CERT);
#endif // ENCRYPTED

    xTaskCreate(&provision_device, PROVISION_DEVICE_TASK_NAME, 1024 * 8, NULL, 5, NULL);
}
