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
#include <string.h>
#include <WalterModem.h>
#include "debug_commands.h"
#include "http_json_example.h"

// Logging tag
static const char *TAG = "walter_nbiot";

// Enable debug mode (set to false to disable verbose debugging)
// WARNING: Debug mode uses a lot of stack memory and may cause overflow
#define DEBUG_MODE false

// Enable JSON test transmission (disable to save memory)
#define ENABLE_JSON_TEST false

// Network configuration - Soracom
#define CELLULAR_APN "soracom.io"          // Soracom APN
#define CELLULAR_APN_USER "sora"           // Soracom username
#define CELLULAR_APN_PASS "sora"           // Soracom password
#define SIM_PIN NULL                       // SIM PIN code (NULL if no PIN)

// Connection timeouts in milliseconds
#define NETWORK_TIMEOUT_MS 180000          // 3 minutes (increased for NB-IoT)
#define ATTACH_TIMEOUT_MS 60000            // 1 minute
#define CHECK_INTERVAL_MS 5000             // 5 seconds (reduced frequency)

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
    WalterModemNetworkRegState regState = WALTER_MODEM_NETWORK_REG_NOT_SEARCHING;
    
    while (elapsed < timeout_ms) {
        regState = modem.getNetworkRegState();
        
        if (regState == WALTER_MODEM_NETWORK_REG_REGISTERED_HOME || 
            regState == WALTER_MODEM_NETWORK_REG_REGISTERED_ROAMING) {
            ESP_LOGI(TAG, "Registered on network");
            return true;
        }
        
        vTaskDelay(pdMS_TO_TICKS(CHECK_INTERVAL_MS));
        elapsed += CHECK_INTERVAL_MS;
    }
    
    ESP_LOGE(TAG, "Network registration timeout (state: %d)", (int)regState);
    return false;
}


/**
 * Get and display signal quality information
 */
static void get_signal_info(void)
{
    WalterModemRsp rsp = {};
    
    if (modem.getSignalQuality(&rsp)) {
        ESP_LOGI(TAG, "Signal quality - RSRP: %d dBm, RSRQ: %d dB", 
                 rsp.data.signalQuality.rsrp, 
                 rsp.data.signalQuality.rsrq);
    } else {
        ESP_LOGW(TAG, "Could not retrieve signal quality");
    }
}


// Global modem instance (not static so it can be used in debug_commands.h)
WalterModem modem;

// Forward declarations removed - functions are defined in debug_commands.h

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
    if (!modem.checkComm()) {
        ESP_LOGE(TAG, "Cannot communicate with modem");
        return false;
    }
    ESP_LOGI(TAG, "OK: Communication established");
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // Run full diagnostics if debug mode is enabled
    if (DEBUG_MODE) {
        run_modem_diagnostics();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
    // Step 3: Get modem identity
    ESP_LOGI(TAG, "[3/10] Getting modem identity...");
    rsp = {};
    if (modem.getIdentity(&rsp)) {
        ESP_LOGI(TAG, "Modem IMEI: %s", rsp.data.identity.imei);
        ESP_LOGI(TAG, "Modem IMEISV: %s", rsp.data.identity.imeisv);
        ESP_LOGI(TAG, "Modem SVN: %s", rsp.data.identity.svn);
    }
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // Step 3.5: Check current operational state
    ESP_LOGI(TAG, "[3.5/10] Checking current operational state...");
    rsp = {};
    if (modem.getOpState(&rsp)) {
        ESP_LOGI(TAG, "Current operational state: %d", rsp.data.opState);
    }
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // Step 3.6: Check current RAT
    ESP_LOGI(TAG, "[3.6/10] Checking current RAT...");
    rsp = {};
    if (modem.getRAT(&rsp)) {
        ESP_LOGI(TAG, "Current RAT: %d (0=CAT-M1, 1=NB-IoT, 2=GSM)", rsp.data.rat);
    } else {
        ESP_LOGW(TAG, "Could not get current RAT");
    }
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // Step 3.7: Check radio bands
    ESP_LOGI(TAG, "[3.7/10] Checking radio bands...");
    rsp = {};
    if (modem.getRadioBands(&rsp)) {
        ESP_LOGI(TAG, "Radio bands configured");
    }
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // Step 4: Set operational state to MINIMUM (required before changing RAT)
    ESP_LOGI(TAG, "[4/10] Setting operational state to MINIMUM...");
    if (!modem.setOpState(WALTER_MODEM_OPSTATE_MINIMUM)) {
        ESP_LOGE(TAG, "Failed to set operational state to MINIMUM");
        return false;
    }
    ESP_LOGI(TAG, "OK: Operational state set to MINIMUM");
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // Step 5: Configure RAT to NB-IoT
    ESP_LOGI(TAG, "[5/10] Configuring RAT to NB-IoT...");
    
    if (DEBUG_MODE) {
        check_rat_support();
    }
    
    // Use debug function for detailed RAT setting
    if (DEBUG_MODE) {
        if (!debug_set_rat(WALTER_MODEM_RAT_NBIOT)) {
            ESP_LOGE(TAG, "Failed to set RAT to NB-IoT");
            ESP_LOGI(TAG, "Trying LTE-M (CAT-M1) as fallback...");
            if (!debug_set_rat(WALTER_MODEM_RAT_LTEM)) {
                ESP_LOGE(TAG, "Failed to set RAT to LTE-M");
                ESP_LOGW(TAG, "Continuing with current RAT setting");
            } else {
                ESP_LOGI(TAG, "OK: RAT set to LTE-M");
            }
        } else {
            ESP_LOGI(TAG, "OK: RAT set to NB-IoT");
        }
    } else {
        // Normal mode (less verbose)
        rsp = {};
        if (!modem.setRAT(WALTER_MODEM_RAT_NBIOT, &rsp)) {
            ESP_LOGE(TAG, "Failed to set RAT to NB-IoT (error code: %d)", rsp.result);
            
            ESP_LOGI(TAG, "Trying LTE-M (CAT-M1) as fallback...");
            rsp = {};
            if (!modem.setRAT(WALTER_MODEM_RAT_LTEM, &rsp)) {
                ESP_LOGE(TAG, "Failed to set RAT to LTE-M (error code: %d)", rsp.result);
                ESP_LOGW(TAG, "Continuing anyway - modem may use default RAT");
            } else {
                ESP_LOGI(TAG, "OK: RAT set to LTE-M");
            }
        } else {
            ESP_LOGI(TAG, "OK: RAT set to NB-IoT");
        }
    }
    
    // Verify final RAT setting
    vTaskDelay(pdMS_TO_TICKS(1000));
    rsp = {};
    if (modem.getRAT(&rsp)) {
        ESP_LOGI(TAG, "Final RAT configuration: %d", rsp.data.rat);
    }
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // Step 5.5: Set operational state back to FULL
    ESP_LOGI(TAG, "[5.5/10] Setting operational state to FULL...");
    if (!modem.setOpState(WALTER_MODEM_OPSTATE_FULL)) {
        ESP_LOGE(TAG, "Failed to set operational state to FULL");
        return false;
    }
    ESP_LOGI(TAG, "OK: Operational state set to FULL");
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // Step 6: Unlock SIM card (skip if no PIN)
    #if SIM_PIN != NULL
    if (strlen(SIM_PIN) > 0) {
        ESP_LOGI(TAG, "[6/10] Unlocking SIM card...");
        if (!modem.unlockSIM(SIM_PIN)) {
            ESP_LOGE(TAG, "Failed to unlock SIM");
            ESP_LOGE(TAG, "Check SIM card and PIN code");
            return false;
        }
        ESP_LOGI(TAG, "OK: SIM unlocked");
    } else
    #endif
    {
        ESP_LOGI(TAG, "[6/10] No SIM PIN required, skipping unlock");
    }
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // Step 6.5: Check SIM state
    ESP_LOGI(TAG, "[6.5/10] Checking SIM state...");
    rsp = {};
    if (modem.getSIMState(&rsp)) {
        ESP_LOGI(TAG, "SIM state: %d", rsp.data.simState);
    }
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // Step 7: Set network selection mode
    ESP_LOGI(TAG, "[7/10] Setting network selection to automatic...");
    if (!modem.setNetworkSelectionMode(WALTER_MODEM_NETWORK_SEL_MODE_AUTOMATIC)) {
        ESP_LOGE(TAG, "Failed to set network selection mode");
        return false;
    }
    ESP_LOGI(TAG, "OK: Network selection mode set");
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // Step 8: Wait for network registration
    ESP_LOGI(TAG, "[8/10] Waiting for network registration...");
    
    if (DEBUG_MODE) {
        check_network_coverage();
    }
    
    if (!wait_for_network_registration(NETWORK_TIMEOUT_MS)) {
        // Get diagnostic info before failing
        ESP_LOGE(TAG, "Network registration failed - gathering diagnostic info:");
        
        if (DEBUG_MODE) {
            check_network_coverage();
            check_rat_support();
        }
        
        rsp = {};
        if (modem.getRAT(&rsp)) {
            ESP_LOGE(TAG, "  Current RAT: %d", rsp.data.rat);
        }
        
        rsp = {};
        if (modem.getSIMState(&rsp)) {
            ESP_LOGE(TAG, "  SIM state: %d", rsp.data.simState);
        }
        
        get_signal_info();
        
        ESP_LOGE(TAG, "");
        ESP_LOGE(TAG, "TROUBLESHOOTING TIPS:");
        ESP_LOGE(TAG, "1. Check antenna connection");
        ESP_LOGE(TAG, "2. Verify NB-IoT/LTE-M coverage in your area");
        ESP_LOGE(TAG, "3. Confirm SIM card is activated in Soracom console");
        ESP_LOGE(TAG, "4. Check if SIM supports NB-IoT or LTE-M");
        ESP_LOGE(TAG, "5. Try moving to a location with better signal");
        
        return false;
    }
    
    // Get signal quality
    get_signal_info();
    
    // Get cell information
    ESP_LOGI(TAG, "Getting cell information...");
    rsp = {};
    if (modem.getCellInformation(WALTER_MODEM_SQNMONI_REPORTS_SERVING_CELL, &rsp)) {
        ESP_LOGI(TAG, "Connected to network");
    }
    
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // Step 9: Define PDP context
    ESP_LOGI(TAG, "[9/10] Defining PDP context...");
    if (!modem.definePDPContext(PDP_CONTEXT_ID, CELLULAR_APN)) {
        ESP_LOGE(TAG, "Failed to define PDP context");
        ESP_LOGE(TAG, "Check APN configuration");
        return false;
    }
    ESP_LOGI(TAG, "OK: PDP context defined");
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // Step 9.5: Set authentication parameters if needed
    if (strlen(CELLULAR_APN_USER) > 0) {
        ESP_LOGI(TAG, "[9.5/10] Setting PDP authentication...");
        if (!modem.setPDPAuthParams(
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
    if (!modem.setPDPContextActive(true)) {
        ESP_LOGE(TAG, "Failed to activate PDP context");
        return false;
    }
    ESP_LOGI(TAG, "OK: PDP context activated");
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // Step 10: Attach to network
    ESP_LOGI(TAG, "[10/10] Attaching to packet domain...");
    if (!modem.setNetworkAttachmentState(true)) {
        ESP_LOGE(TAG, "Failed to attach to network");
        return false;
    }
    
    // Wait for attachment to complete
    vTaskDelay(pdMS_TO_TICKS(5000));
    ESP_LOGI(TAG, "OK: Attached to network");
    
    // Get PDP address
    ESP_LOGI(TAG, "Getting IP address...");
    rsp = {};
    if (modem.getPDPAddress(&rsp)) {
        ESP_LOGI(TAG, "PDP Context ID: %d", rsp.data.pdpAddressList.pdpCtxId);
        
        if (rsp.data.pdpAddressList.pdpAddress != NULL && 
            rsp.data.pdpAddressList.pdpAddress[0] != '\0') {
            ESP_LOGI(TAG, "Primary IP Address: %s", rsp.data.pdpAddressList.pdpAddress);
        } else {
            ESP_LOGI(TAG, "Primary IP Address: None");
        }
        
        if (rsp.data.pdpAddressList.pdpAddress2 != NULL && 
            rsp.data.pdpAddressList.pdpAddress2[0] != '\0') {
            ESP_LOGI(TAG, "Secondary IP Address: %s", rsp.data.pdpAddressList.pdpAddress2);
        }
    } else {
        ESP_LOGW(TAG, "Could not retrieve IP address");
    }
    
    ESP_LOGI(TAG, "==================================================");
    ESP_LOGI(TAG, "CONNECTION SUCCESSFUL!");
    ESP_LOGI(TAG, "==================================================");
    ESP_LOGI(TAG, "Modem is ready for data transmission");
    
    return true;
}


/**
 * Test JSON transmission
 */
__attribute__((unused)) static void test_json_transmission(void)
{
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "==================================================");
    ESP_LOGI(TAG, "Testing JSON Transmission");
    ESP_LOGI(TAG, "==================================================");
    
    // Wait a bit before sending
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // Example 1: Send to httpbin.org (test server)
    ESP_LOGI(TAG, "Example 1: Sending telemetry to test server...");
    if (send_telemetry_example()) {
        ESP_LOGI(TAG, "✓ Telemetry sent successfully!");
    } else {
        ESP_LOGE(TAG, "✗ Failed to send telemetry");
    }
    
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    // Example 2: Send sensor data
    ESP_LOGI(TAG, "Example 2: Sending sensor data...");
    if (send_sensor_data_example("http://httpbin.org/post")) {
        ESP_LOGI(TAG, "✓ Sensor data sent successfully!");
    } else {
        ESP_LOGE(TAG, "✗ Failed to send sensor data");
    }
    
    ESP_LOGI(TAG, "==================================================");
    ESP_LOGI(TAG, "JSON Transmission Test Complete");
    ESP_LOGI(TAG, "==================================================");
}


/**
 * Monitor connection status task (minimal version to prevent stack overflow)
 */
static void monitor_task(void *pvParameters)
{
    const TickType_t xDelay = pdMS_TO_TICKS(60000); // Check every 60 seconds
    
    while (1) {
        vTaskDelay(xDelay);
        
        // Minimal status check - just verify we're still registered
        WalterModemNetworkRegState regState = modem.getNetworkRegState();
        
        if (regState != WALTER_MODEM_NETWORK_REG_REGISTERED_HOME && 
            regState != WALTER_MODEM_NETWORK_REG_REGISTERED_ROAMING) {
            // Only log if there's a problem
            ESP_LOGW(TAG, "Network lost: %d", regState);
        }
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
    
    // Test JSON transmission (optional - can be disabled to save memory)
    #if ENABLE_JSON_TEST
    test_json_transmission();
    #else
    ESP_LOGI(TAG, "JSON test disabled (ENABLE_JSON_TEST=false)");
    #endif
    
    // Create monitoring task with minimal stack (optimized)
    BaseType_t taskCreated = xTaskCreate(
        monitor_task,
        "monitor",       // Shorter name
        2048,            // Minimal stack - just checks network state
        NULL,
        1,               // Lower priority
        NULL
    );
    
    if (taskCreated != pdPASS) {
        ESP_LOGE(TAG, "Failed to create monitoring task");
    }
    
    // Main loop - keep alive
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000)); // Just keep alive, don't spam
        
        // Uncomment to send periodic data:
        /*
        ESP_LOGI(TAG, "Sending periodic data...");
        char* json = create_custom_json("walter-001", 24.5, 62.0);
        if (json != NULL) {
            send_json_http("http://httpbin.org/post", json);
            cJSON_free(json);
        }
        */
    }
}
