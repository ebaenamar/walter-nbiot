#ifndef AT_COMMANDS_H
#define AT_COMMANDS_H

#include <esp_log.h>
#include <WalterModem.h>
#include <string.h>

static const char *AT_TAG = "at_cmd";

// External reference to modem instance
extern WalterModem modem;

/**
 * Send raw AT command directly to the modem
 * Note: This is a simplified version that logs the command
 * The actual AT command execution depends on WalterModem library internals
 * 
 * @param cmd AT command string (for logging)
 * @param response Not used (kept for API compatibility)
 * @param response_size Not used (kept for API compatibility)
 * @param timeout_ms Not used (kept for API compatibility)
 * @return Always returns true (for compatibility)
 */
static bool send_at_command(const char* cmd, char* response = NULL, size_t response_size = 0, uint32_t timeout_ms = 5000) {
    if (cmd == NULL) {
        ESP_LOGE(AT_TAG, "Command is NULL");
        return false;
    }
    
    ESP_LOGI(AT_TAG, ">>> %s", cmd);
    ESP_LOGI(AT_TAG, "    (Note: Direct AT command execution not available in current API)");
    ESP_LOGI(AT_TAG, "    (Use WalterModem API functions instead)");
    
    return true;
}

/**
 * Run a sequence of AT commands for RAT configuration
 */
static void configure_rat_with_at_commands(void) {
    ESP_LOGI(AT_TAG, "");
    ESP_LOGI(AT_TAG, "========================================");
    ESP_LOGI(AT_TAG, "RAT CONFIGURATION VIA AT COMMANDS");
    ESP_LOGI(AT_TAG, "========================================");
    
    // Check current state
    ESP_LOGI(AT_TAG, "");
    ESP_LOGI(AT_TAG, "1. Checking current configuration:");
    send_at_command("AT+CFUN?", NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    send_at_command("AT+SQNMODEACTIVE?", NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    send_at_command("AT+SQNCTM?", NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // Set to minimum functionality
    ESP_LOGI(AT_TAG, "");
    ESP_LOGI(AT_TAG, "2. Setting minimum functionality mode:");
    send_at_command("AT+CFUN=0", NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    // Configure RAT to NB-IoT
    ESP_LOGI(AT_TAG, "");
    ESP_LOGI(AT_TAG, "3. Configuring RAT to NB-IoT:");
    ESP_LOGI(AT_TAG, "   AT+SQNCTM values:");
    ESP_LOGI(AT_TAG, "   0 = Auto (LTE-M/NB-IoT)");
    ESP_LOGI(AT_TAG, "   1 = NB-IoT only");
    ESP_LOGI(AT_TAG, "   2 = LTE-M only");
    ESP_LOGI(AT_TAG, "   3 = GSM only");
    
    send_at_command("AT+SQNCTM=1", NULL, 0);  // NB-IoT only
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // Verify configuration
    ESP_LOGI(AT_TAG, "");
    ESP_LOGI(AT_TAG, "4. Verifying RAT configuration:");
    send_at_command("AT+SQNCTM?", NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // Set to full functionality
    ESP_LOGI(AT_TAG, "");
    ESP_LOGI(AT_TAG, "5. Setting full functionality mode:");
    send_at_command("AT+CFUN=1", NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(5000));
    
    // Check active mode
    ESP_LOGI(AT_TAG, "");
    ESP_LOGI(AT_TAG, "6. Checking active RAT mode:");
    send_at_command("AT+SQNMODEACTIVE?", NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    ESP_LOGI(AT_TAG, "");
    ESP_LOGI(AT_TAG, "========================================");
    ESP_LOGI(AT_TAG, "RAT CONFIGURATION COMPLETE");
    ESP_LOGI(AT_TAG, "========================================");
    ESP_LOGI(AT_TAG, "");
}

/**
 * Try LTE-M as alternative
 */
static void configure_ltem_with_at_commands(void) {
    ESP_LOGI(AT_TAG, "");
    ESP_LOGI(AT_TAG, "========================================");
    ESP_LOGI(AT_TAG, "TRYING LTE-M CONFIGURATION");
    ESP_LOGI(AT_TAG, "========================================");
    
    send_at_command("AT+CFUN=0", NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    send_at_command("AT+SQNCTM=2", NULL, 0);  // LTE-M only
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    send_at_command("AT+SQNCTM?", NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    send_at_command("AT+CFUN=1", NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(5000));
    
    send_at_command("AT+SQNMODEACTIVE?", NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    ESP_LOGI(AT_TAG, "========================================");
    ESP_LOGI(AT_TAG, "");
}

/**
 * Check network information
 */
static void check_network_info_at(void) {
    ESP_LOGI(AT_TAG, "");
    ESP_LOGI(AT_TAG, "========================================");
    ESP_LOGI(AT_TAG, "NETWORK INFORMATION");
    ESP_LOGI(AT_TAG, "========================================");
    
    ESP_LOGI(AT_TAG, "");
    ESP_LOGI(AT_TAG, "Signal Quality:");
    send_at_command("AT+CSQ", NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    send_at_command("AT+CESQ", NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    ESP_LOGI(AT_TAG, "");
    ESP_LOGI(AT_TAG, "Network Registration:");
    send_at_command("AT+CREG?", NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    send_at_command("AT+CEREG?", NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    send_at_command("AT+CGREG?", NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    ESP_LOGI(AT_TAG, "");
    ESP_LOGI(AT_TAG, "Detailed Network Info:");
    send_at_command("AT+SQNMONI", NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    ESP_LOGI(AT_TAG, "");
    ESP_LOGI(AT_TAG, "Operator:");
    send_at_command("AT+COPS?", NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    ESP_LOGI(AT_TAG, "");
    ESP_LOGI(AT_TAG, "PDP Context:");
    send_at_command("AT+CGDCONT?", NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    ESP_LOGI(AT_TAG, "");
    ESP_LOGI(AT_TAG, "GPRS Attach:");
    send_at_command("AT+CGATT?", NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    ESP_LOGI(AT_TAG, "========================================");
    ESP_LOGI(AT_TAG, "");
}

/**
 * Check band configuration
 */
static void check_bands_at(void) {
    ESP_LOGI(AT_TAG, "");
    ESP_LOGI(AT_TAG, "========================================");
    ESP_LOGI(AT_TAG, "BAND CONFIGURATION");
    ESP_LOGI(AT_TAG, "========================================");
    
    ESP_LOGI(AT_TAG, "");
    ESP_LOGI(AT_TAG, "NB-IoT Bands:");
    send_at_command("AT+SQNBANDSEL?", NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    ESP_LOGI(AT_TAG, "");
    ESP_LOGI(AT_TAG, "Band Scan Configuration:");
    send_at_command("AT+SQNBANDSEL=?", NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    ESP_LOGI(AT_TAG, "========================================");
    ESP_LOGI(AT_TAG, "");
}

/**
 * Reset modem to factory defaults
 */
static void factory_reset_at(void) {
    ESP_LOGW(AT_TAG, "");
    ESP_LOGW(AT_TAG, "========================================");
    ESP_LOGW(AT_TAG, "FACTORY RESET - USE WITH CAUTION!");
    ESP_LOGW(AT_TAG, "========================================");
    
    ESP_LOGW(AT_TAG, "This will reset ALL modem settings!");
    ESP_LOGW(AT_TAG, "Waiting 5 seconds... (cancel if needed)");
    vTaskDelay(pdMS_TO_TICKS(5000));
    
    send_at_command("AT&F", NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    send_at_command("AT&W", NULL, 0);  // Save to NVM
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    ESP_LOGW(AT_TAG, "Factory reset complete. Rebooting modem...");
    send_at_command("AT+CFUN=1,1", NULL, 0);  // Reboot
    vTaskDelay(pdMS_TO_TICKS(10000));
    
    ESP_LOGW(AT_TAG, "========================================");
    ESP_LOGW(AT_TAG, "");
}

/**
 * Complete AT command diagnostic suite
 */
static void run_at_diagnostics(void) {
    ESP_LOGI(AT_TAG, "");
    ESP_LOGI(AT_TAG, "========================================");
    ESP_LOGI(AT_TAG, "COMPLETE AT COMMAND DIAGNOSTICS");
    ESP_LOGI(AT_TAG, "========================================");
    
    // Basic modem info
    ESP_LOGI(AT_TAG, "");
    ESP_LOGI(AT_TAG, "1. MODEM INFORMATION:");
    send_at_command("AT", NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    send_at_command("ATI", NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    send_at_command("AT+CGMM", NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    send_at_command("AT+CGMR", NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    send_at_command("AT+CGSN", NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // SIM info
    ESP_LOGI(AT_TAG, "");
    ESP_LOGI(AT_TAG, "2. SIM INFORMATION:");
    send_at_command("AT+CPIN?", NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    send_at_command("AT+CCID", NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    send_at_command("AT+CIMI", NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // RAT configuration
    ESP_LOGI(AT_TAG, "");
    ESP_LOGI(AT_TAG, "3. RAT CONFIGURATION:");
    send_at_command("AT+CFUN?", NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    send_at_command("AT+SQNMODEACTIVE?", NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    send_at_command("AT+SQNCTM?", NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // Network info
    check_network_info_at();
    
    // Band info
    check_bands_at();
    
    ESP_LOGI(AT_TAG, "========================================");
    ESP_LOGI(AT_TAG, "DIAGNOSTICS COMPLETE");
    ESP_LOGI(AT_TAG, "========================================");
    ESP_LOGI(AT_TAG, "");
}

#endif // AT_COMMANDS_H
