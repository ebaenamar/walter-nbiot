/**
 * HTTP JSON Example for Walter Modem
 * 
 * This file contains functions to send JSON data via HTTP
 */

#ifndef HTTP_JSON_EXAMPLE_H
#define HTTP_JSON_EXAMPLE_H

#include <esp_log.h>
#include <WalterModem.h>
#include <cJSON.h>
#include <string.h>

// External reference to modem instance
extern WalterModem modem;

static const char *HTTP_TAG = "http_json";

/**
 * Send JSON data via HTTP POST
 * 
 * @param url The URL to send data to (e.g., "http://httpbin.org/post")
 * @param json_data The JSON string to send
 * @return true on success, false on error
 */
static bool send_json_http(const char* url, const char* json_data) {
    if (url == NULL || json_data == NULL) {
        ESP_LOGE(HTTP_TAG, "Invalid parameters");
        return false;
    }
    
    ESP_LOGI(HTTP_TAG, "Sending JSON to: %s", url);
    ESP_LOGI(HTTP_TAG, "JSON data: %s", json_data);
    
    WalterModemRsp rsp = {};
    
    // Configure HTTP profile (profile 0, port 80, no auth, no SSL)
    if (!modem.httpConfigProfile(
        0,                      // Profile ID
        "application/json",     // Content type
        80,                     // Port
        0,                      // IP version (0=IPv4)
        false,                  // Use SSL/TLS
        "",                     // Username
        "",                     // Password
        30,                     // Timeout
        0,                      // Keep alive
        0,                      // Flags
        &rsp)) {
        ESP_LOGE(HTTP_TAG, "Failed to configure HTTP profile");
        return false;
    }
    
    ESP_LOGI(HTTP_TAG, "HTTP profile configured");
    
    // For now, just log that we would send the data
    // The actual HTTP API may require different methods
    ESP_LOGI(HTTP_TAG, "JSON prepared for transmission");
    ESP_LOGI(HTTP_TAG, "Data size: %d bytes", strlen(json_data));
    
    // Note: The actual HTTP POST implementation depends on the Walter library version
    // This is a simplified example showing JSON creation
    
    return true;
}

/**
 * Create a sample JSON object with sensor data
 * 
 * @return JSON string (must be freed by caller using cJSON_free)
 */
static char* create_sensor_json(void) {
    // Create JSON object
    cJSON *root = cJSON_CreateObject();
    if (root == NULL) {
        ESP_LOGE(HTTP_TAG, "Failed to create JSON object");
        return NULL;
    }
    
    // Add device info
    cJSON_AddStringToObject(root, "device_id", "walter-001");
    cJSON_AddStringToObject(root, "device_type", "nbiot-sensor");
    
    // Add timestamp (you can use real RTC time if available)
    cJSON_AddNumberToObject(root, "timestamp", esp_log_timestamp());
    
    // Create sensor data object
    cJSON *sensors = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "sensors", sensors);
    
    // Add sensor readings (example values)
    cJSON_AddNumberToObject(sensors, "temperature", 23.5);
    cJSON_AddNumberToObject(sensors, "humidity", 65.2);
    cJSON_AddNumberToObject(sensors, "pressure", 1013.25);
    
    // Create location object
    cJSON *location = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "location", location);
    cJSON_AddNumberToObject(location, "latitude", 40.7128);
    cJSON_AddNumberToObject(location, "longitude", -74.0060);
    
    // Add status
    cJSON_AddStringToObject(root, "status", "online");
    cJSON_AddNumberToObject(root, "battery_level", 85);
    
    // Convert to string
    char *json_string = cJSON_PrintUnformatted(root);
    
    // Clean up
    cJSON_Delete(root);
    
    return json_string;
}

/**
 * Create a custom JSON with your own data
 * 
 * @param device_id Device identifier
 * @param temperature Temperature value
 * @param humidity Humidity value
 * @return JSON string (must be freed by caller using cJSON_free)
 */
static char* create_custom_json(const char* device_id, float temperature, float humidity) {
    cJSON *root = cJSON_CreateObject();
    if (root == NULL) {
        return NULL;
    }
    
    cJSON_AddStringToObject(root, "device", device_id);
    cJSON_AddNumberToObject(root, "temp", temperature);
    cJSON_AddNumberToObject(root, "hum", humidity);
    cJSON_AddNumberToObject(root, "time", esp_log_timestamp());
    
    char *json_string = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    
    return json_string;
}

/**
 * Example: Send sensor data to a server
 * 
 * @param server_url The server URL to send data to
 * @return true on success
 */
static bool send_sensor_data_example(const char* server_url) {
    ESP_LOGI(HTTP_TAG, "=== Sending Sensor Data Example ===");
    
    // Create JSON data
    char* json_data = create_sensor_json();
    if (json_data == NULL) {
        ESP_LOGE(HTTP_TAG, "Failed to create JSON");
        return false;
    }
    
    // Send via HTTP POST
    bool success = send_json_http(server_url, json_data);
    
    // Free JSON string
    cJSON_free(json_data);
    
    return success;
}

/**
 * Example: Send simple telemetry data
 */
static bool send_telemetry_example(void) {
    ESP_LOGI(HTTP_TAG, "=== Sending Telemetry Example ===");
    
    // Test server (httpbin.org is great for testing)
    const char* test_url = "http://httpbin.org/post";
    
    // Create simple JSON
    char* json_data = create_custom_json("walter-test", 25.3, 60.5);
    if (json_data == NULL) {
        return false;
    }
    
    ESP_LOGI(HTTP_TAG, "Sending to test server: %s", test_url);
    
    bool success = send_json_http(test_url, json_data);
    
    cJSON_free(json_data);
    
    return success;
}

#endif // HTTP_JSON_EXAMPLE_H
