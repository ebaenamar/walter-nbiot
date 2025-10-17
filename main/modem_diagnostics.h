#ifndef MODEM_DIAGNOSTICS_H
#define MODEM_DIAGNOSTICS_H

#include <esp_log.h>
#include <WalterModem.h>

static const char *DIAG_TAG = "modem_diag";

// External reference to modem instance
extern WalterModem modem;

/**
 * Log AT command (for documentation purposes)
 * Note: We can't easily send raw AT commands with the current API
 */
static void log_at_command(const char* cmd, const char* description) {
    ESP_LOGI(DIAG_TAG, "%s: %s", description, cmd);
}

/**
 * Complete modem diagnostic suite using WalterModem API
 */
static void run_complete_diagnostics(void) {
    ESP_LOGI(DIAG_TAG, "");
    ESP_LOGI(DIAG_TAG, "========================================");
    ESP_LOGI(DIAG_TAG, "COMPLETE MODEM DIAGNOSTICS");
    ESP_LOGI(DIAG_TAG, "========================================");
    
    WalterModemRsp rsp = {};
    
    // 1. Modem Identity
    ESP_LOGI(DIAG_TAG, "");
    ESP_LOGI(DIAG_TAG, "1. MODEM IDENTIFICATION:");
    ESP_LOGI(DIAG_TAG, "------------------------");
    
    rsp = {};
    if (modem.getIdentity(&rsp)) {
        ESP_LOGI(DIAG_TAG, "IMEI: %s", rsp.data.identity.imei);
        ESP_LOGI(DIAG_TAG, "IMEISV: %s", rsp.data.identity.imeisv);
        ESP_LOGI(DIAG_TAG, "SVN: %s", rsp.data.identity.svn);
    } else {
        ESP_LOGE(DIAG_TAG, "Failed to get modem identity");
    }
    
    // 2. SIM Card Status
    ESP_LOGI(DIAG_TAG, "");
    ESP_LOGI(DIAG_TAG, "2. SIM CARD STATUS:");
    ESP_LOGI(DIAG_TAG, "-------------------");
    
    rsp = {};
    if (modem.getSIMState(&rsp)) {
        ESP_LOGI(DIAG_TAG, "SIM State: %d (%s)", rsp.data.simState,
                 rsp.data.simState == WALTER_MODEM_SIM_STATE_READY ? "READY" :
                 rsp.data.simState == WALTER_MODEM_SIM_STATE_PIN_REQUIRED ? "PIN REQUIRED" :
                 rsp.data.simState == WALTER_MODEM_SIM_STATE_PUK_REQUIRED ? "PUK REQUIRED" : "UNKNOWN");
    } else {
        ESP_LOGE(DIAG_TAG, "Failed to get SIM state");
    }
    
    rsp = {};
    if (modem.getSIMCardID(&rsp)) {
        ESP_LOGI(DIAG_TAG, "SIM ICCID retrieved successfully");
    }
    
    // 3. RAT Configuration (CRITICAL!)
    ESP_LOGI(DIAG_TAG, "");
    ESP_LOGI(DIAG_TAG, "3. RAT CONFIGURATION (CRITICAL):");
    ESP_LOGI(DIAG_TAG, "--------------------------------");
    
    rsp = {};
    if (modem.getRAT(&rsp)) {
        ESP_LOGI(DIAG_TAG, "Current RAT: %d (%s)", rsp.data.rat,
                 rsp.data.rat == WALTER_MODEM_RAT_NBIOT ? "NB-IoT" :
                 rsp.data.rat == WALTER_MODEM_RAT_LTEM ? "LTE-M" :
                 rsp.data.rat == WALTER_MODEM_RAT_AUTO ? "Auto" : "UNKNOWN/ERROR");
                 
        ESP_LOGI(DIAG_TAG, "Expected values:");
        ESP_LOGI(DIAG_TAG, "  WALTER_MODEM_RAT_NBIOT = 8");
        ESP_LOGI(DIAG_TAG, "  WALTER_MODEM_RAT_LTEM = 9");
        ESP_LOGI(DIAG_TAG, "  WALTER_MODEM_RAT_AUTO = 0");
        
        if (rsp.data.rat != WALTER_MODEM_RAT_NBIOT && rsp.data.rat != WALTER_MODEM_RAT_LTEM) {
            ESP_LOGW(DIAG_TAG, "WARNING: RAT is NOT set to NB-IoT or LTE-M!");
            ESP_LOGW(DIAG_TAG, "This will prevent NB-IoT/LTE-M connection!");
        }
    } else {
        ESP_LOGE(DIAG_TAG, "Failed to get RAT");
    }
    
    // 4. Operational State
    ESP_LOGI(DIAG_TAG, "");
    ESP_LOGI(DIAG_TAG, "4. OPERATIONAL STATE:");
    ESP_LOGI(DIAG_TAG, "---------------------");
    
    rsp = {};
    if (modem.getOpState(&rsp)) {
        ESP_LOGI(DIAG_TAG, "Operational State: %d (%s)", rsp.data.opState,
                 rsp.data.opState == WALTER_MODEM_OPSTATE_MINIMUM ? "MINIMUM" :
                 rsp.data.opState == WALTER_MODEM_OPSTATE_FULL ? "FULL" :
                 rsp.data.opState == WALTER_MODEM_OPSTATE_NO_RF ? "NO_RF" : "UNKNOWN");
    }
    
    // 5. Signal Quality
    ESP_LOGI(DIAG_TAG, "");
    ESP_LOGI(DIAG_TAG, "5. SIGNAL QUALITY:");
    ESP_LOGI(DIAG_TAG, "------------------");
    
    rsp = {};
    if (modem.getSignalQuality(&rsp)) {
        int16_t rsrp = rsp.data.signalQuality.rsrp;
        int16_t rsrq = rsp.data.signalQuality.rsrq;
        
        ESP_LOGI(DIAG_TAG, "RSRP: %d dBm (should be -80 to -140)", rsrp);
        ESP_LOGI(DIAG_TAG, "RSRQ: %d dB (should be -3 to -20)", rsrq);
        
        if (rsrp > 0 || rsrp < -150) {
            ESP_LOGW(DIAG_TAG, "WARNING: RSRP value is invalid! Modem may not be ready.");
        }
        if (rsrq > 0 || rsrq < -50) {
            ESP_LOGW(DIAG_TAG, "WARNING: RSRQ value is invalid! Modem may not be ready.");
        }
    } else {
        ESP_LOGE(DIAG_TAG, "Failed to get signal quality");
    }
    
    // 6. Network Registration
    ESP_LOGI(DIAG_TAG, "");
    ESP_LOGI(DIAG_TAG, "6. NETWORK REGISTRATION:");
    ESP_LOGI(DIAG_TAG, "------------------------");
    
    WalterModemNetworkRegState regState = modem.getNetworkRegState();
    ESP_LOGI(DIAG_TAG, "Registration State: %d (%s)", regState,
             regState == WALTER_MODEM_NETWORK_REG_NOT_SEARCHING ? "NOT_SEARCHING" :
             regState == WALTER_MODEM_NETWORK_REG_REGISTERED_HOME ? "REGISTERED_HOME" :
             regState == WALTER_MODEM_NETWORK_REG_SEARCHING ? "SEARCHING" :
             regState == WALTER_MODEM_NETWORK_REG_DENIED ? "DENIED" :
             regState == WALTER_MODEM_NETWORK_REG_REGISTERED_ROAMING ? "REGISTERED_ROAMING" : "UNKNOWN");
    
    // 7. Radio Bands
    ESP_LOGI(DIAG_TAG, "");
    ESP_LOGI(DIAG_TAG, "7. RADIO BANDS:");
    ESP_LOGI(DIAG_TAG, "---------------");
    
    rsp = {};
    if (modem.getRadioBands(&rsp)) {
        ESP_LOGI(DIAG_TAG, "Radio bands configured successfully");
    } else {
        ESP_LOGW(DIAG_TAG, "Could not get radio bands");
    }
    
    ESP_LOGI(DIAG_TAG, "");
    ESP_LOGI(DIAG_TAG, "========================================");
    ESP_LOGI(DIAG_TAG, "DIAGNOSTICS COMPLETE");
    ESP_LOGI(DIAG_TAG, "========================================");
    ESP_LOGI(DIAG_TAG, "");
    ESP_LOGI(DIAG_TAG, "IMPORTANT AT COMMANDS TO CHECK MANUALLY:");
    ESP_LOGI(DIAG_TAG, "  AT+SQNMODEACTIVE? - Shows active RAT mode");
    ESP_LOGI(DIAG_TAG, "  AT+SQNCTM? - Shows RAT configuration");
    ESP_LOGI(DIAG_TAG, "  AT+SQNMONI - Shows detailed network info");
    ESP_LOGI(DIAG_TAG, "");
}


#endif // MODEM_DIAGNOSTICS_H
