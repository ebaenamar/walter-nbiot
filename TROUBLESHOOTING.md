# Troubleshooting Guide

## Common Issues and Solutions

### Issue 1: Build Error - `regState` not declared

**Error Message:**
```
error: 'regState' was not declared in this scope
```

**Cause:** 
The variable `regState` was used in the ESP_LOGE statement outside its scope.

**Solution:**
Initialize the variable at declaration:
```cpp
WalterModemNetworkRegState regState = WALTER_MODEM_NETWORK_REG_NOT_SEARCHING;
```

This has been fixed in the latest version.

---

### Issue 2: Failed to set RAT (Radio Access Technology)

**Error Message:**
```
E (6636) walter_nbiot: Failed to set RAT to NB-IoT
I (6636) walter_nbiot: Trying LTE-M as fallback...  
E (6666) walter_nbiot: Failed to set RAT to LTE-M
```

**Root Cause:**
The modem must be in **MINIMUM** operational state before changing the RAT. Setting RAT while in FULL state will fail.

**Solution Applied:**
The code now follows this sequence:
1. Set operational state to **MINIMUM**
2. Change RAT to NB-IoT or LTE-M
3. Set operational state back to **FULL**

**Code Changes:**
```cpp
// Step 4: Set to MINIMUM first
WalterModem::setOpState(WALTER_MODEM_OPSTATE_MINIMUM);
vTaskDelay(pdMS_TO_TICKS(2000));

// Step 5: Now change RAT
WalterModem::setRAT(WALTER_MODEM_RAT_NBIOT);
vTaskDelay(pdMS_TO_TICKS(2000));

// Step 5.5: Set back to FULL
WalterModem::setOpState(WALTER_MODEM_OPSTATE_FULL);
vTaskDelay(pdMS_TO_TICKS(2000));
```

---

### Issue 3: Soracom APN Configuration

**Soracom Default Settings:**

For standard Soracom Air SIM cards:
```cpp
#define CELLULAR_APN "soracom.io"
#define CELLULAR_APN_USER "sora"
#define CELLULAR_APN_PASS "sora"
```

**Alternative Soracom APNs:**

Depending on your Soracom plan and region, you might need:

- **Global Coverage**: `soracom.io`
- **Japan (plan01s)**: `soracom.io`
- **Custom APN**: Check your Soracom console for custom APN settings

**Authentication:**
- Soracom requires PAP authentication
- Username: `sora`
- Password: `sora`

**Reference:**
https://support.soracom.io/hc/en-us/articles/218427317

---

### Issue 4: Network Registration Fails

**Symptoms:**
- Modem connects but doesn't register on network
- Timeout at step 8

**Possible Causes:**

1. **Coverage Issue**
   - Check if you have NB-IoT or LTE-M coverage in your area
   - Try moving to a location with better signal

2. **SIM Card Not Activated**
   - Verify SIM is activated in Soracom console
   - Check subscription status

3. **Wrong RAT**
   - If NB-IoT fails, the code will try LTE-M
   - Check which technology your carrier supports

4. **Antenna Issue**
   - Ensure antenna is properly connected
   - Check antenna is suitable for cellular frequencies

**Debugging Steps:**

1. Check signal quality:
   ```
   Signal quality - RSSI: -XX dBm, RSRP: -XX dBm
   ```
   - RSSI > -100 dBm is good
   - RSRP > -110 dBm is acceptable

2. Check SIM state:
   ```
   SIM state: X
   ```
   - Should be READY (value depends on enum)

3. Enable verbose logging:
   ```cpp
   esp_log_level_set("walter_nbiot", ESP_LOG_DEBUG);
   ```

---

### Issue 5: PDP Context Activation Fails

**Symptoms:**
- Network registration succeeds
- PDP context activation fails

**Solutions:**

1. **Verify APN Settings**
   - Double-check APN name
   - Verify username/password

2. **Check Authentication Protocol**
   - Soracom uses PAP (already configured)
   - Some carriers use CHAP

3. **Wait Longer**
   - Some networks need more time
   - Increase delays between steps

---

### Issue 6: UART Communication Issues

**Symptoms:**
- Modem initialization fails
- Cannot communicate with modem

**Solutions:**

1. **Check UART Number**
   ```cpp
   #define MODEM_UART_NUM UART_NUM_1  // Try UART_NUM_0 or UART_NUM_2
   ```

2. **Verify Hardware Connections**
   - TX/RX pins properly connected
   - Power supply adequate (modem needs significant current)

3. **Check Baud Rate**
   - Default is usually 115200
   - Verify in WalterModem library settings

---

## Debugging Tips

### Enable Debug Logging

Add at the start of `app_main()`:
```cpp
esp_log_level_set("*", ESP_LOG_INFO);
esp_log_level_set("walter_nbiot", ESP_LOG_DEBUG);
esp_log_level_set("WalterModem", ESP_LOG_DEBUG);
```

### Monitor AT Commands

The WalterModem library sends AT commands. To see them:
1. Check library documentation for debug mode
2. Use a logic analyzer on UART lines
3. Enable modem library verbose mode if available

### Check Modem Firmware

Some issues may be firmware-related:
```cpp
WalterModemRsp rsp = {};
if (WalterModem::getIdentity(&rsp)) {
    ESP_LOGI(TAG, "Modem firmware: %s", rsp.data.identity);
}
```

### Test with AT Commands Directly

You can send raw AT commands:
```cpp
WalterModem::sendCmd("AT+COPS?");  // Check operator
WalterModem::sendCmd("AT+CEREG?"); // Check registration
WalterModem::sendCmd("AT+CSQ");    // Check signal quality
```

---

## Hardware Checklist

- [ ] SIM card properly inserted
- [ ] Antenna connected
- [ ] USB cable provides enough power (or use external power)
- [ ] Board is Walter (ESP32-S3 based)
- [ ] No physical damage to board

## Software Checklist

- [ ] ESP-IDF version 5.0 or later
- [ ] Target set to esp32s3
- [ ] WalterModem component installed
- [ ] Correct APN configured
- [ ] Correct UART number configured

## Soracom-Specific Checklist

- [ ] SIM activated in Soracom console
- [ ] Subscription active
- [ ] Coverage available in your region
- [ ] APN set to `soracom.io`
- [ ] Username/password set to `sora`/`sora`

---

## Getting Help

If issues persist:

1. **Check Logs**
   - Save full serial output
   - Note exact error messages

2. **Verify Hardware**
   - Test with known working SIM
   - Try different antenna

3. **Community Support**
   - Walter GitHub issues: https://github.com/QuickSpot/walter-esp-idf/issues
   - Soracom support: https://support.soracom.io/

4. **Provide Information**
   - ESP-IDF version
   - WalterModem library version
   - Full error log
   - Location/carrier information
