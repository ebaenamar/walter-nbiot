# Stack Configuration Guide

## 📊 Stack Sizes Explained

### What is Stack?
Stack is memory used by tasks to store:
- Local variables
- Function call information
- Return addresses
- Function parameters

### Default ESP-IDF Stack Sizes
- **Main task:** 3584 bytes (3.5 KB) - Too small!
- **FreeRTOS idle task:** 1536 bytes
- **System event task:** 2048 bytes

## ⚠️ Stack Overflow Problem

### Symptoms
```
***ERROR*** A stack overflow in task X has been detected.
Guru Meditation Error: Core 0 panic'ed (Unhandled debug exception)
```

### Common Causes
1. **Deep function call chains** - Each function call uses stack
2. **Large local variables** - Arrays, structs on stack
3. **Recursive functions** - Each recursion level uses more stack
4. **String formatting** - `ESP_LOGI`, `sprintf` use stack buffers
5. **Debug mode** - Extra logging uses more stack

## ✅ Solution: Increase Stack Sizes

### Method 1: sdkconfig.defaults (Recommended)

Create `sdkconfig.defaults` file:

```ini
# Main task stack size (16KB instead of default 3.5KB)
CONFIG_ESP_MAIN_TASK_STACK_SIZE=16384

# Enable stack overflow detection
CONFIG_FREERTOS_WATCHPOINT_END_OF_STACK=y

# System event task stack
CONFIG_ESP_SYSTEM_EVENT_TASK_STACK_SIZE=4096
```

**After creating this file:**
```bash
rm -rf build sdkconfig
idf.py build
```

### Method 2: xTaskCreate() for Custom Tasks

```cpp
// Monitor task with 4KB stack
xTaskCreate(
    monitor_task,
    "monitor",
    4096,        // Stack size in bytes
    NULL,
    5,           // Priority
    NULL
);
```

### Method 3: menuconfig

```bash
idf.py menuconfig
```

Navigate to:
```
Component config → ESP System Settings → Main task stack size
```

Change from 3584 to 16384

## 📏 Recommended Stack Sizes

| Task Type | Stack Size | Use Case |
|-----------|-----------|----------|
| Main task | 16384 (16KB) | Complex initialization, networking |
| Monitor task | 4096 (4KB) | Periodic checks, simple logging |
| Simple task | 2048 (2KB) | Basic operations, no logging |
| Heavy task | 8192 (8KB) | JSON parsing, HTTP requests |

## 🔍 How to Check Stack Usage

### Method 1: Runtime Check
```cpp
UBaseType_t stackHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
ESP_LOGI(TAG, "Stack free: %d bytes", stackHighWaterMark);
```

### Method 2: Enable Stack Monitoring
```cpp
// In sdkconfig.defaults
CONFIG_FREERTOS_WATCHPOINT_END_OF_STACK=y
```

This will detect overflows immediately and show which task failed.

## 💡 Stack Optimization Tips

### 1. Move Large Variables to Heap
```cpp
// ❌ Bad - Uses stack
char buffer[1024];

// ✅ Good - Uses heap
char* buffer = (char*)malloc(1024);
// ... use buffer ...
free(buffer);
```

### 2. Use Static for Large Arrays
```cpp
// ❌ Bad - Uses stack
void myFunction() {
    char bigArray[2048];
}

// ✅ Good - Uses static memory
void myFunction() {
    static char bigArray[2048];
}
```

### 3. Reduce String Formatting
```cpp
// ❌ Bad - Uses lots of stack
ESP_LOGI(TAG, "Very long message with %d %s %f...", a, b, c);

// ✅ Good - Minimal stack
ESP_LOGI(TAG, "Msg: %d", value);
```

### 4. Avoid Deep Recursion
```cpp
// ❌ Bad - Each recursion uses stack
void recursive(int n) {
    if (n > 0) recursive(n - 1);
}

// ✅ Good - Use iteration
void iterative(int n) {
    for (int i = 0; i < n; i++) {
        // ...
    }
}
```

## 🎯 Walter NB-IoT Specific

### Our Configuration

**sdkconfig.defaults:**
```ini
CONFIG_ESP_MAIN_TASK_STACK_SIZE=16384  # 16KB for main task
```

**main.cpp:**
```cpp
// Monitor task: 4KB
xTaskCreate(monitor_task, "monitor", 4096, NULL, 5, NULL);
```

### Why These Sizes?

1. **Main task (16KB):**
   - Modem initialization
   - Network connection
   - JSON creation
   - HTTP requests
   - Debug logging

2. **Monitor task (4KB):**
   - Simple network checks
   - Minimal logging
   - No heavy operations

## 🐛 Debugging Stack Issues

### 1. Enable Stack Overflow Detection
```ini
CONFIG_FREERTOS_WATCHPOINT_END_OF_STACK=y
```

### 2. Check Stack Usage
```cpp
void printTaskStats() {
    char buffer[512];
    vTaskList(buffer);
    ESP_LOGI(TAG, "Task stats:\n%s", buffer);
}
```

### 3. Monitor Free Heap
```cpp
ESP_LOGI(TAG, "Free heap: %d bytes", esp_get_free_heap_size());
ESP_LOGI(TAG, "Min free heap: %d bytes", esp_get_minimum_free_heap_size());
```

## 📊 Memory Budget

ESP32-S3 has plenty of RAM:
- **Total SRAM:** 512 KB
- **PSRAM (optional):** Up to 8 MB

Our usage:
- Main task: 16 KB
- Monitor task: 4 KB
- Modem library: ~20 KB
- System: ~30 KB
- **Total:** ~70 KB (14% of available RAM)

**We have plenty of room!** Don't be afraid to use larger stacks.

## ✅ Final Configuration

**sdkconfig.defaults:**
```ini
CONFIG_ESP_MAIN_TASK_STACK_SIZE=16384
CONFIG_FREERTOS_WATCHPOINT_END_OF_STACK=y
CONFIG_ESP_SYSTEM_EVENT_TASK_STACK_SIZE=4096
```

**main.cpp:**
```cpp
// Monitor task
xTaskCreate(monitor_task, "monitor", 4096, NULL, 5, NULL);
```

**Result:**
- ✅ No more stack overflow
- ✅ Can enable DEBUG_MODE
- ✅ Can enable JSON_TEST
- ✅ Stable operation

## 🚀 Apply Configuration

```bash
# Clean build
rm -rf build sdkconfig

# Build with new config
idf.py build

# Flash
idf.py flash monitor
```

## 📝 Notes

1. **Stack is cheap** - ESP32-S3 has 512KB RAM
2. **Overflow is expensive** - Crashes and debugging time
3. **Start big, optimize later** - Begin with generous stacks
4. **Monitor in production** - Use `uxTaskGetStackHighWaterMark()`

**When in doubt, use more stack!** 🎯
