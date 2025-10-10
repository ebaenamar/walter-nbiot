/**
 * Walter NB-IoT Connection Test - ESP-IDF Version
 * 
 * This application demonstrates how to connect to a 5G NB-IoT network
 * using the Walter modem board with ESP-IDF framework.
 * 
 * Setup:
 *   1. Create ESP-IDF project from template
 *   2. Install component: idf.py add-dependency "dptechnics/walter-modem^1.1.3"
 *   3. Set target: ESP-IDF: Set Espressif Device Target -> esp32s3
 *   4. Configure APN settings below
 *   5. Build and flash: idf.py build flash monitor
 */

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <WalterModem.h>

// Logging tag
static const char *TAG = "walter_nbiot";

// Network configuration - Soracom
#define CELLULAR_APN "soracom.io"          // Soracom APN
#define CELLULAR_APN_USER "sora"           // Soracom username
#define CELLULAR_APN_PASS "sora"           // Soracom password
#define SIM_PIN NULL                       // SIM PIN code (NULL if no PIN)

// Connection timeouts in milliseconds
#define NETWORK_TIMEOUT_MS 120000          // 2 minutes
#define ATTACH_TIMEOUT_MS 60000            // 1 minute
#define CHECK_INTERVAL_MS 2000             // 2 seconds

// UART configuration for modem
#define MODEM_UART_NUM UART_NUM_1

// PDP Context ID
#define PDP_CONTEXT_ID 1


/**
 * Wait for network registration with timeout
 */
static bool wait_for_network_registration(uint32_t timeout_ms)
{
    ESP_LOGI(TAG, "Waiting for network registration");
    
    uint32_t elapsed = 0;
    WalterModemNetworkRegState regState;
    
    while (elapsed < timeout_ms) {
        regState = WalterModem::getNetworkRegState();
        
        if (regState == WALTER_MODEM_NETWORK_REG_REGISTERED_HOME || 
            regState == WALTER_MODEM_NETWORK_REG_REGISTERED_ROAMING) {
            ESP_LOGI(TAG, "Registered on network");
            return true;
        }
        
        vTaskDelay(pdMS_TO_TICKS(CHECK_INTERVAL_MS));
        elapsed += CHECK_INTERVAL_MS;
    }
    
    ESP_LOGE(TAG, "Network registration timeout (state: %d)", regState);
    return false;
}


/**
 * Get and display signal quality information
 */
static void get_signal_info(void)
{
    WalterModemRsp rsp = {};
    
    if (WalterModem::getSignalQuality(&rsp)) {
        ESP_LOGI(TAG, "Signal quality - RSSI: %d dBm, RSRP: %d dBm", 
                 rsp.data.signalQuality.rssi, 
                 rsp.data.signalQuality.rsrp);
    } else {
        ESP_LOGW(TAG, "Could not retrieve signal quality");
    }
}


/**
 * Main NB-IoT connection function
 */
static bool connect_nbiot(void)
{
    WalterModemRsp rsp = {};
    
    ESP_LOGI(TAG, "==================================================");
    ESP_LOGI(TAG, "Walter NB-IoT Connection Test - ESP-IDF");
    ESP_LOGI(TAG, "==================================================");
    
    // Step 1: Initialize modem
    ESP_LOGI(TAG, "[1/10] Initializing modem...");
    if (!WalterModem::begin(MODEM_UART_NUM)) {
        ESP_LOGE(TAG, "Failed to initialize modem");
        ESP_LOGE(TAG, "Check hardware connections and restart");
        return false;
    }
    ESP_LOGI(TAG, "OK: Modem initialized");
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // Step 2: Check communication
    ESP_LOGI(TAG, "[2/10] Checking modem communication...");
    if (!WalterModem::checkComm()) {
        ESP_LOGE(TAG, "Cannot communicate with modem");
        return false;
    }
    ESP_LOGI(TAG, "OK: Communication established");
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // Step 3: Get modem identity
    ESP_LOGI(TAG, "[3/10] Getting modem identity...");
    rsp = {};
    if (WalterModem::getIdentity(&rsp)) {
        ESP_LOGI(TAG, "Modem info: %s", rsp.data.identity);
    }
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // Step 4: Set operational state to FULL
    ESP_LOGI(TAG, "[4/10] Setting operational state to FULL...");
    if (!WalterModem::setOpState(WALTER_MODEM_OPSTATE_FULL)) {
        ESP_LOGE(TAG, "Failed to set operational state");
        return false;
    }
    ESP_LOGI(TAG, "OK: Operational state set");
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // Step 5: Configure RAT to NB-IoT
    ESP_LOGI(TAG, "[5/10] Configuring RAT to NB-IoT...");
    if (!WalterModem::setRAT(WALTER_MODEM_RAT_NBIOT)) {
        ESP_LOGE(TAG, "Failed to set RAT to NB-IoT");
        ESP_LOGI(TAG, "Trying LTE-M as fallback...");
        if (!WalterModem::setRAT(WALTER_MODEM_RAT_LTEM)) {
            ESP_LOGE(TAG, "Failed to set RAT to LTE-M");
            return false;
        }
        ESP_LOGI(TAG, "OK: RAT set to LTE-M");
    } else {
        ESP_LOGI(TAG, "OK: RAT set to NB-IoT");
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // Step 6: Unlock SIM card
    ESP_LOGI(TAG, "[6/10] Unlocking SIM card...");
    if (!WalterModem::unlockSIM(SIM_PIN)) {
        ESP_LOGE(TAG, "Failed to unlock SIM");
        ESP_LOGE(TAG, "Check SIM card and PIN code");
        return false;
    }
    ESP_LOGI(TAG, "OK: SIM unlocked");
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // Step 6.5: Check SIM state
    ESP_LOGI(TAG, "[6.5/10] Checking SIM state...");
    rsp = {};
    if (WalterModem::getSIMState(&rsp)) {
        ESP_LOGI(TAG, "SIM state: %d", rsp.data.simState);
    }
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // Step 7: Set network selection mode
    ESP_LOGI(TAG, "[7/10] Setting network selection to automatic...");
    if (!WalterModem::setNetworkSelectionMode(WALTER_MODEM_NETWORK_SEL_MODE_AUTOMATIC)) {
        ESP_LOGE(TAG, "Failed to set network selection mode");
        return false;
    }
    ESP_LOGI(TAG, "OK: Network selection mode set");
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // Step 8: Wait for network registration
    ESP_LOGI(TAG, "[8/10] Waiting for network registration...");
    if (!wait_for_network_registration(NETWORK_TIMEOUT_MS)) {
        return false;
    }
    
    // Get signal quality
    get_signal_info();
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // Step 9: Define PDP context
    ESP_LOGI(TAG, "[9/10] Defining PDP context...");
    if (!WalterModem::definePDPContext(PDP_CONTEXT_ID, CELLULAR_APN)) {
        ESP_LOGE(TAG, "Failed to define PDP context");
        ESP_LOGE(TAG, "Check APN configuration");
        return false;
    }
    ESP_LOGI(TAG, "OK: PDP context defined");
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // Step 9.5: Set authentication parameters if needed
    if (CELLULAR_APN_USER != NULL && strlen(CELLULAR_APN_USER) > 0) {
        ESP_LOGI(TAG, "[9.5/10] Setting PDP authentication...");
        if (!WalterModem::setPDPAuthParams(
                WALTER_MODEM_PDP_AUTH_PROTO_PAP, 
                CELLULAR_APN_USER, 
                CELLULAR_APN_PASS)) {
            ESP_LOGW(TAG, "Failed to set authentication parameters");
        } else {
            ESP_LOGI(TAG, "OK: Authentication parameters set");
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    
    // Step 9.6: Activate PDP context
    ESP_LOGI(TAG, "[9.6/10] Activating PDP context...");
    if (!WalterModem::setPDPContextActive(true)) {
        ESP_LOGE(TAG, "Failed to activate PDP context");
        return false;
    }
    ESP_LOGI(TAG, "OK: PDP context activated");
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // Step 10: Attach to network
    ESP_LOGI(TAG, "[10/10] Attaching to packet domain...");
    if (!WalterModem::setNetworkAttachmentState(true)) {
        ESP_LOGE(TAG, "Failed to attach to network");
        return false;
    }
    
    // Wait for attachment to complete
    vTaskDelay(pdMS_TO_TICKS(5000));
    ESP_LOGI(TAG, "OK: Attached to network");
    
    // Get PDP address
    ESP_LOGI(TAG, "Getting IP address...");
    rsp = {};
    if (WalterModem::getPDPAddress(&rsp)) {
        ESP_LOGI(TAG, "PDP Context ID: %d", rsp.data.pdpAddressList.pdpCtxId);
        
        if (rsp.data.pdpAddressList.pdpAddress && 
            rsp.data.pdpAddressList.pdpAddress[0] != '\0') {
            ESP_LOGI(TAG, "Primary IP Address: %s", rsp.data.pdpAddressList.pdpAddress);
        } else {
            ESP_LOGI(TAG, "Primary IP Address: None");
        }
        
        if (rsp.data.pdpAddressList.pdpAddress2 && 
            rsp.data.pdpAddressList.pdpAddress2[0] != '\0') {
            ESP_LOGI(TAG, "Secondary IP Address: %s", rsp.data.pdpAddressList.pdpAddress2);
        }
    } else {
        ESP_LOGW(TAG, "Could not retrieve IP address");
    }
    
    ESP_LOGI(TAG, "==================================================");
    ESP_LOGI(TAG, "CONNECTION SUCCESSFUL!");
    ESP_LOGI(TAG, "==================================================");
    ESP_LOGI(TAG, "You can now use the modem for data transmission.");
    ESP_LOGI(TAG, "Check the WalterModem library examples for HTTP, MQTT, and Socket usage.");
    
    return true;
}


/**
 * Monitor connection status task
 */
static void monitor_task(void *pvParameters)
{
    const TickType_t xDelay = pdMS_TO_TICKS(30000); // 30 seconds
    
    while (1) {
        vTaskDelay(xDelay);
        
        ESP_LOGI(TAG, "--- Status Check ---");
        
        // Check network registration
        WalterModemNetworkRegState regState = WalterModem::getNetworkRegState();
        ESP_LOGI(TAG, "Network registration: %s", 
            regState == WALTER_MODEM_NETWORK_REG_NOT_SEARCHING ? "Not searching" :
            regState == WALTER_MODEM_NETWORK_REG_REGISTERED_HOME ? "Registered (Home)" :
            regState == WALTER_MODEM_NETWORK_REG_SEARCHING ? "Searching..." :
            regState == WALTER_MODEM_NETWORK_REG_DENIED ? "Denied" :
            regState == WALTER_MODEM_NETWORK_REG_REGISTERED_ROAMING ? "Registered (Roaming)" :
            "Unknown");
        
        // Check signal quality
        get_signal_info();
        
        ESP_LOGI(TAG, "-------------------");
    }
}


/**
 * Main application entry point
 */
extern "C" void app_main(void)
{
    // Connect to NB-IoT network
    if (!connect_nbiot()) {
        ESP_LOGE(TAG, "Connection failed. Please check configuration and restart.");
        return;
    }
    
    // Create monitoring task
    xTaskCreate(
        monitor_task,
        "monitor_task",
        4096,
        NULL,
        5,
        NULL
    );
    
    // Main task can do other work here
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
