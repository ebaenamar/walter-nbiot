/**
 * Debug Commands for Walter Modem
 * 
 * This file contains helper functions to send AT commands directly
 * for debugging purposes.
 */

#ifndef DEBUG_COMMANDS_H
#define DEBUG_COMMANDS_H

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <WalterModem.h>

// External reference to modem instance (defined in main.cpp)
extern WalterModem modem;

static const char *DEBUG_TAG = "walter_debug";

/**
 * Send a raw AT command and log the response
 */
static void send_debug_command(const char* cmd, const char* description) {
    if (cmd == NULL || description == NULL) {
        ESP_LOGE(DEBUG_TAG, "Invalid parameters to send_debug_command");
        return;
    }
    
    ESP_LOGI(DEBUG_TAG, "Sending: %s (%s)", cmd, description);
    WalterModemRsp rsp = {};
    if (modem.sendCmd(cmd, NULL, &rsp)) {
        ESP_LOGI(DEBUG_TAG, "  Response OK");
    } else {
        ESP_LOGE(DEBUG_TAG, "  Response FAILED");
    }
}

/**
 * Run comprehensive modem diagnostics
 */
static void run_modem_diagnostics(void) {
    ESP_LOGI(DEBUG_TAG, "========================================");
    ESP_LOGI(DEBUG_TAG, "Running Modem Diagnostics");
    ESP_LOGI(DEBUG_TAG, "========================================");
    
    // Basic modem info
    send_debug_command("ATI", "Modem identification");
    send_debug_command("AT+CGMR", "Firmware version");
    send_debug_command("AT+CGSN", "IMEI");
    
    // SIM card info
    send_debug_command("AT+CIMI", "IMSI");
    send_debug_command("AT+CCID", "SIM ICCID");
    send_debug_command("AT+CPIN?", "SIM PIN status");
    
    // Network info
    send_debug_command("AT+COPS?", "Current operator");
    send_debug_command("AT+CEREG?", "Network registration status");
    send_debug_command("AT+CSQ", "Signal quality");
    
    // RAT configuration
    send_debug_command("AT+URAT?", "Current RAT setting");
    send_debug_command("AT+UBANDMASK?", "Band mask");
    
    // Operational state
    send_debug_command("AT+CFUN?", "Functionality level");
    
    // PDP context
    send_debug_command("AT+CGDCONT?", "PDP context definition");
    send_debug_command("AT+CGACT?", "PDP context activation state");
    send_debug_command("AT+CGATT?", "GPRS attachment state");
    
    ESP_LOGI(DEBUG_TAG, "========================================");
    ESP_LOGI(DEBUG_TAG, "Diagnostics Complete");
    ESP_LOGI(DEBUG_TAG, "========================================");
}

/**
 * Check specific RAT support
 */
static void check_rat_support(void) {
    ESP_LOGI(DEBUG_TAG, "Checking RAT support:");
    
    WalterModemRsp rsp = {};
    
    // Try to get supported RATs
    send_debug_command("AT+URAT=?", "Supported RAT values");
    
    // Check current RAT
    if (modem.getRAT(&rsp)) {
        const char* rat_name = "Unknown";
        switch(rsp.data.rat) {
            case WALTER_MODEM_RAT_LTEM:
                rat_name = "LTE-M (CAT-M1)";
                break;
            case WALTER_MODEM_RAT_NBIOT:
                rat_name = "NB-IoT";
                break;
            case WALTER_MODEM_RAT_GSM:
                rat_name = "GSM";
                break;
        }
        ESP_LOGI(DEBUG_TAG, "Current RAT: %d (%s)", rsp.data.rat, rat_name);
    } else {
        ESP_LOGE(DEBUG_TAG, "Failed to get current RAT");
    }
}

/**
 * Check network coverage
 */
static void check_network_coverage(void) {
    ESP_LOGI(DEBUG_TAG, "Checking network coverage:");
    
    // Signal quality
    WalterModemRsp rsp = {};
    if (modem.getSignalQuality(&rsp)) {
        ESP_LOGI(DEBUG_TAG, "  RSSI: %d dBm", rsp.data.signalQuality.rssi);
        ESP_LOGI(DEBUG_TAG, "  RSRP: %d dBm", rsp.data.signalQuality.rsrp);
        ESP_LOGI(DEBUG_TAG, "  RSRQ: %d dB", rsp.data.signalQuality.rsrq);
        ESP_LOGI(DEBUG_TAG, "  SNR: %d dB", rsp.data.signalQuality.snr);
        
        // Interpret signal quality
        if (rsp.data.signalQuality.rsrp > -80) {
            ESP_LOGI(DEBUG_TAG, "  Signal: EXCELLENT");
        } else if (rsp.data.signalQuality.rsrp > -90) {
            ESP_LOGI(DEBUG_TAG, "  Signal: GOOD");
        } else if (rsp.data.signalQuality.rsrp > -100) {
            ESP_LOGI(DEBUG_TAG, "  Signal: FAIR");
        } else if (rsp.data.signalQuality.rsrp > -110) {
            ESP_LOGI(DEBUG_TAG, "  Signal: POOR");
        } else {
            ESP_LOGI(DEBUG_TAG, "  Signal: VERY POOR");
        }
    } else {
        ESP_LOGE(DEBUG_TAG, "Failed to get signal quality");
    }
    
    // Network registration state
    WalterModemNetworkRegState regState = modem.getNetworkRegState();
    const char* reg_name = "Unknown";
    switch(regState) {
        case WALTER_MODEM_NETWORK_REG_NOT_SEARCHING:
            reg_name = "Not searching";
            break;
        case WALTER_MODEM_NETWORK_REG_REGISTERED_HOME:
            reg_name = "Registered (Home)";
            break;
        case WALTER_MODEM_NETWORK_REG_SEARCHING:
            reg_name = "Searching";
            break;
        case WALTER_MODEM_NETWORK_REG_DENIED:
            reg_name = "Registration denied";
            break;
        case WALTER_MODEM_NETWORK_REG_REGISTERED_ROAMING:
            reg_name = "Registered (Roaming)";
            break;
    }
    ESP_LOGI(DEBUG_TAG, "  Registration: %s (%d)", reg_name, regState);
}

/**
 * Try to manually set RAT with detailed logging
 */
static bool debug_set_rat(WalterModemRat rat) {
    const char* rat_name = (rat == WALTER_MODEM_RAT_NBIOT) ? "NB-IoT" : 
                           (rat == WALTER_MODEM_RAT_LTEM) ? "LTE-M" : "Unknown";
    
    ESP_LOGI(DEBUG_TAG, "Attempting to set RAT to %s (%d)", rat_name, rat);
    
    // Check current state
    WalterModemRsp rsp = {};
    if (modem.getOpState(&rsp)) {
        ESP_LOGI(DEBUG_TAG, "  Current op state: %d", rsp.data.opState);
    }
    
    // Try to set RAT
    rsp = {};
    bool result = modem.setRAT(rat, &rsp);
    
    if (result) {
        ESP_LOGI(DEBUG_TAG, "  RAT set successfully");
    } else {
        ESP_LOGE(DEBUG_TAG, "  RAT set FAILED (result: %d)", rsp.result);
    }
    
    // Verify what was actually set
    vTaskDelay(pdMS_TO_TICKS(1000));
    rsp = {};
    if (modem.getRAT(&rsp)) {
        ESP_LOGI(DEBUG_TAG, "  Verified RAT: %d", rsp.data.rat);
    }
    
    return result;
}

#endif // DEBUG_COMMANDS_H
