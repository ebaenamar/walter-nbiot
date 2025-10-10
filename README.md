# Walter NB-IoT Connection Test - ESP-IDF

Complete ESP-IDF project for connecting a Walter board to a 5G NB-IoT network.

## Hardware Requirements

- DPTechnics Walter board (ESP32-S3 based)
- NB-IoT compatible SIM card
- Antenna connected to the board
- USB cable for programming and monitoring

## Software Requirements

1. **ESP-IDF** (version 5.0 or later)
2. **VS Code** with ESP-IDF extension installed
3. **Walter Modem Component** (automatically installed via component manager)

## Project Setup

### Step 1: Open Project in VS Code

Open this folder in VS Code with the ESP-IDF extension installed.

### Step 2: Set Device Target

Open the VS Code command palette (Cmd+Shift+P or Ctrl+Shift+P) and run:

```
ESP-IDF: Set Espressif Device Target
```

Select: **esp32s3**

### Step 3: Install Dependencies

The Walter Modem component will be automatically installed when you build the project.
It's defined in `main/idf_component.yml`.

Alternatively, you can manually install it via ESP-IDF Terminal:

```bash
idf.py add-dependency "dptechnics/walter-modem^1.1.3"
```

### Step 4: Configure APN Settings

Edit `main/main.cpp` and update these values:

```cpp
#define CELLULAR_APN ""           // Your operator's APN (REQUIRED)
#define CELLULAR_APN_USER ""      // APN username (empty if not required)
#define CELLULAR_APN_PASS ""      // APN password (empty if not required)
#define SIM_PIN NULL              // SIM PIN code (NULL if no PIN)
```

### Finding Your APN

Contact your NB-IoT network operator to get the correct APN. Common examples:

- **Vodafone**: `nb.iot.vodafone.com`
- **T-Mobile**: `iot.1nce.net` (for 1NCE SIM cards)
- **AT&T**: `m2m.com.attz`

## Build and Flash

### Using VS Code

1. Click the **Build** button in the status bar (or use command palette: `ESP-IDF: Build your project`)
2. Click the **Flash** button to upload to the board
3. Click the **Monitor** button to view serial output

### Using Terminal

Open ESP-IDF Terminal and run:

```bash
# Build the project
idf.py build

# Flash to the board
idf.py flash

# Monitor serial output
idf.py monitor

# Or do all three in one command
idf.py build flash monitor
```

To exit the monitor, press `Ctrl+]`

## Expected Output

The application will show 10 steps:

```
I (xxx) walter_nbiot: ==================================================
I (xxx) walter_nbiot: Walter NB-IoT Connection Test - ESP-IDF
I (xxx) walter_nbiot: ==================================================
I (xxx) walter_nbiot: [1/10] Initializing modem...
I (xxx) walter_nbiot: OK: Modem initialized
I (xxx) walter_nbiot: [2/10] Checking modem communication...
I (xxx) walter_nbiot: OK: Communication established
I (xxx) walter_nbiot: [3/10] Getting modem identity...
I (xxx) walter_nbiot: Modem info: ...
I (xxx) walter_nbiot: [4/10] Setting operational state to FULL...
I (xxx) walter_nbiot: OK: Operational state set
I (xxx) walter_nbiot: [5/10] Configuring RAT to NB-IoT...
I (xxx) walter_nbiot: OK: RAT set to NB-IoT
I (xxx) walter_nbiot: [6/10] Unlocking SIM card...
I (xxx) walter_nbiot: OK: SIM unlocked
I (xxx) walter_nbiot: [6.5/10] Checking SIM state...
I (xxx) walter_nbiot: SIM state: ...
I (xxx) walter_nbiot: [7/10] Setting network selection to automatic...
I (xxx) walter_nbiot: OK: Network selection mode set
I (xxx) walter_nbiot: [8/10] Waiting for network registration...
I (xxx) walter_nbiot: Registered on network
I (xxx) walter_nbiot: Signal quality - RSSI: -XX dBm, RSRP: -XX dBm
I (xxx) walter_nbiot: [9/10] Defining PDP context...
I (xxx) walter_nbiot: OK: PDP context defined
I (xxx) walter_nbiot: [9.6/10] Activating PDP context...
I (xxx) walter_nbiot: OK: PDP context activated
I (xxx) walter_nbiot: [10/10] Attaching to packet domain...
I (xxx) walter_nbiot: OK: Attached to network
I (xxx) walter_nbiot: Getting IP address...
I (xxx) walter_nbiot: PDP Context ID: 1
I (xxx) walter_nbiot: Primary IP Address: XXX.XXX.XXX.XXX
I (xxx) walter_nbiot: ==================================================
I (xxx) walter_nbiot: CONNECTION SUCCESSFUL!
I (xxx) walter_nbiot: ==================================================
```

After connection, the application monitors the connection status every 30 seconds.

## Project Structure

```
walter-nbiot-espidf/
├── CMakeLists.txt              # Top-level CMake configuration
├── README.md                   # This file
└── main/
    ├── CMakeLists.txt          # Main component CMake
    ├── idf_component.yml       # Component dependencies
    └── main.cpp                # Main application code
```

## Troubleshooting

### Build Errors

**Error: Component not found**
- Ensure you're connected to the internet (component manager needs to download dependencies)
- Try: `idf.py reconfigure`

**Error: Target not set**
- Run: `ESP-IDF: Set Espressif Device Target` and select `esp32s3`

### Runtime Errors

**Modem initialization fails**
- Check USB connection
- Verify board is powered correctly
- Try resetting the board

**SIM unlock fails**
- Verify SIM card is inserted correctly
- Check PIN code (if required)
- Ensure SIM is activated by operator

**Network registration timeout**
- Check antenna connection
- Verify NB-IoT coverage in your area
- Confirm SIM card is NB-IoT compatible
- Try LTE-M as fallback (code will attempt this automatically)

**PDP context activation fails**
- Double-check APN configuration
- Verify APN username/password (if required)
- Contact your operator to confirm settings

## Configuration Options

### UART Configuration

The modem uses UART1 by default. To change:

```cpp
#define MODEM_UART_NUM UART_NUM_1  // Change to UART_NUM_0 or UART_NUM_2
```

### Timeouts

Adjust connection timeouts if needed:

```cpp
#define NETWORK_TIMEOUT_MS 120000   // Network registration timeout
#define ATTACH_TIMEOUT_MS 60000     // Attachment timeout
#define CHECK_INTERVAL_MS 2000      // Status check interval
```

### Logging Level

To change log verbosity, use menuconfig:

```bash
idf.py menuconfig
```

Navigate to: `Component config → Log output → Default log verbosity`

Or set per-component in code:

```cpp
esp_log_level_set("walter_nbiot", ESP_LOG_DEBUG);
```

## Monitoring

The application automatically monitors:

- Network registration status (every 30 seconds)
- Signal quality (RSSI and RSRP)

The monitoring runs in a separate FreeRTOS task.

## Next Steps

Once connected, you can:

1. **Send HTTP requests** - Check WalterModem HTTP examples
2. **Use MQTT** - Check WalterModem MQTT examples
3. **Create socket connections** - Check WalterModem Socket examples
4. **Get GPS location** - Check WalterModem GNSS examples

## Additional Resources

- [Walter Documentation](https://github.com/QuickSpot/walter-documentation)
- [WalterModem ESP-IDF Library](https://github.com/QuickSpot/walter-esp-idf)
- [ESP-IDF API Reference](https://github.com/QuickSpot/walter-documentation/blob/main/walter-modem/arduino_esp-idf/reference/reference.md)
- [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/)

## Important Notes

1. **File Extension**: All implementation files must use `.cpp` extension (not `.c`)
2. **Target Device**: Must be set to `esp32s3`
3. **IDF Version**: Requires ESP-IDF 5.0 or later
4. **Component Manager**: Automatically downloads dependencies on first build

## License

This example code is in the public domain.
