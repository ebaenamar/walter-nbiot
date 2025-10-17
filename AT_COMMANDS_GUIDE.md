# Guía de AT Commands para Walter NB-IoT

## 📋 Descripción

Este archivo (`at_commands.h`) proporciona funciones para enviar AT commands directamente al modem Sequans GM02SP en el módulo Walter.

## 🚀 Cómo Usar

### 1. Incluir el Header

```cpp
#include "at_commands.h"
```

### 2. Funciones Disponibles

#### Enviar AT Command Simple

```cpp
// Enviar comando y esperar respuesta
send_at_command("AT+SQNCTM?", NULL, 0);

// Con buffer de respuesta
char response[256];
send_at_command("AT+CGSN", response, sizeof(response));
```

#### Configurar RAT con AT Commands

```cpp
// Configurar NB-IoT
configure_rat_with_at_commands();

// O configurar LTE-M
configure_ltem_with_at_commands();
```

#### Diagnósticos Completos

```cpp
// Ejecutar todos los diagnósticos AT
run_at_diagnostics();

// Solo información de red
check_network_info_at();

// Solo bandas
check_bands_at();
```

## 📡 AT Commands Importantes

### RAT Configuration

| Command | Descripción | Valores |
|---------|-------------|---------|
| `AT+SQNCTM?` | Ver configuración RAT actual | - |
| `AT+SQNCTM=0` | Auto (LTE-M/NB-IoT) | 0 = Auto |
| `AT+SQNCTM=1` | NB-IoT only | 1 = NB-IoT |
| `AT+SQNCTM=2` | LTE-M only | 2 = LTE-M |
| `AT+SQNCTM=3` | GSM only | 3 = GSM |
| `AT+SQNMODEACTIVE?` | Ver RAT activo | - |

### Functionality Mode

| Command | Descripción |
|---------|-------------|
| `AT+CFUN?` | Ver modo funcional |
| `AT+CFUN=0` | Minimum (apaga radio) |
| `AT+CFUN=1` | Full (enciende radio) |
| `AT+CFUN=4` | Airplane mode |
| `AT+CFUN=1,1` | Reboot modem |

### Network Information

| Command | Descripción |
|---------|-------------|
| `AT+CSQ` | Signal quality |
| `AT+CESQ` | Extended signal quality |
| `AT+CREG?` | Network registration (CS) |
| `AT+CEREG?` | EPS registration (PS) |
| `AT+CGREG?` | GPRS registration |
| `AT+COPS?` | Operator selection |
| `AT+SQNMONI` | Detailed network info |

### SIM Card

| Command | Descripción |
|---------|-------------|
| `AT+CPIN?` | SIM PIN status |
| `AT+CCID` | SIM ICCID |
| `AT+CIMI` | SIM IMSI |

### Band Configuration

| Command | Descripción |
|---------|-------------|
| `AT+SQNBANDSEL?` | Ver bandas configuradas |
| `AT+SQNBANDSEL=?` | Ver bandas soportadas |
| `AT+SQNBANDSEL=0,standard,3,5,8,20` | Configurar bandas NB-IoT |

### PDP Context

| Command | Descripción |
|---------|-------------|
| `AT+CGDCONT?` | Ver contextos PDP |
| `AT+CGDCONT=1,"IP","soracom.io"` | Configurar APN |
| `AT+CGATT?` | Ver estado GPRS attach |
| `AT+CGATT=1` | Attach a GPRS |

## 🔧 Ejemplo de Uso en main.cpp

### Opción 1: Agregar al inicio de connect_nbiot()

```cpp
static bool connect_nbiot(void)
{
    WalterModemRsp rsp = {};
    
    // ... inicialización del modem ...
    
    // Ejecutar diagnósticos AT
    run_at_diagnostics();
    
    // Configurar RAT manualmente
    configure_rat_with_at_commands();
    
    // ... resto del código ...
}
```

### Opción 2: Crear función separada

```cpp
// En main.cpp
#include "at_commands.h"

static void debug_with_at_commands(void) {
    ESP_LOGI(TAG, "Starting AT command debugging...");
    
    // Diagnósticos completos
    run_at_diagnostics();
    
    // Intentar configurar NB-IoT
    configure_rat_with_at_commands();
    
    // Verificar resultado
    check_network_info_at();
}

extern "C" void app_main(void)
{
    // Inicializar modem
    if (!modem.begin()) {
        ESP_LOGE(TAG, "Failed to initialize modem");
        return;
    }
    
    // Ejecutar debugging AT
    debug_with_at_commands();
    
    // ... resto del código ...
}
```

### Opción 3: Habilitar con flag

```cpp
// En main.cpp
#define ENABLE_AT_DEBUGGING true

extern "C" void app_main(void)
{
    if (!connect_nbiot()) {
        ESP_LOGE(TAG, "Connection failed");
        
        #if ENABLE_AT_DEBUGGING
        ESP_LOGI(TAG, "Running AT command diagnostics...");
        run_at_diagnostics();
        configure_rat_with_at_commands();
        #endif
        
        return;
    }
}
```

## 🐛 Debugging RAT=1 Problem

### Secuencia Recomendada

```cpp
// 1. Ver configuración actual
send_at_command("AT+SQNCTM?", NULL, 0);
send_at_command("AT+SQNMODEACTIVE?", NULL, 0);

// 2. Apagar radio
send_at_command("AT+CFUN=0", NULL, 0);
vTaskDelay(pdMS_TO_TICKS(3000));

// 3. Configurar NB-IoT
send_at_command("AT+SQNCTM=1", NULL, 0);
vTaskDelay(pdMS_TO_TICKS(1000));

// 4. Verificar
send_at_command("AT+SQNCTM?", NULL, 0);

// 5. Encender radio
send_at_command("AT+CFUN=1", NULL, 0);
vTaskDelay(pdMS_TO_TICKS(5000));

// 6. Verificar RAT activo
send_at_command("AT+SQNMODEACTIVE?", NULL, 0);
```

## 📊 Interpretación de Respuestas

### AT+SQNMODEACTIVE?

```
+SQNMODEACTIVE: 1    // NB-IoT activo ✅
+SQNMODEACTIVE: 2    // LTE-M activo ✅
+SQNMODEACTIVE: 0    // Auto/Unknown ❌
```

### AT+SQNCTM?

```
+SQNCTM: 0    // Auto
+SQNCTM: 1    // NB-IoT configurado ✅
+SQNCTM: 2    // LTE-M configurado ✅
+SQNCTM: 3    // GSM configurado
```

### AT+CEREG?

```
+CEREG: 0,0    // Not registered, not searching
+CEREG: 0,1    // Registered, home network ✅
+CEREG: 0,2    // Not registered, searching
+CEREG: 0,3    // Registration denied ❌
+CEREG: 0,5    // Registered, roaming ✅
```

### AT+CSQ

```
+CSQ: 99,99    // Unknown/No signal ❌
+CSQ: 15,99    // Good signal ✅
+CSQ: 0-31,99  // 0=poor, 31=excellent
```

## ⚠️ Comandos Peligrosos

### Factory Reset

```cpp
// ⚠️ CUIDADO: Borra TODA la configuración
factory_reset_at();

// O manualmente:
send_at_command("AT&F", NULL, 0);      // Factory reset
send_at_command("AT&W", NULL, 0);      // Save to NVM
send_at_command("AT+CFUN=1,1", NULL, 0); // Reboot
```

### Reboot Modem

```cpp
send_at_command("AT+CFUN=1,1", NULL, 0);
vTaskDelay(pdMS_TO_TICKS(10000)); // Wait 10 seconds
```

## 🔍 Troubleshooting

### Si RAT no cambia:

1. Verificar que `CFUN=0` antes de cambiar
2. Esperar suficiente tiempo después de `CFUN=1`
3. Verificar con `AT+SQNMODEACTIVE?` no solo `AT+SQNCTM?`
4. Probar factory reset si nada funciona

### Si no hay señal:

1. Verificar antena conectada
2. Verificar cobertura NB-IoT en tu área
3. Probar diferentes bandas
4. Intentar LTE-M en lugar de NB-IoT

### Si SIM no funciona:

1. Verificar PIN con `AT+CPIN?`
2. Verificar ICCID con `AT+CCID`
3. Verificar que SIM esté activada en Soracom
4. Verificar que SIM soporte NB-IoT/LTE-M

## 📝 Logs Esperados

### Configuración Exitosa

```
I (1234) at_cmd: >>> AT+SQNCTM=1
I (1250) at_cmd: <<< OK
I (2000) at_cmd: >>> AT+CFUN=1
I (2100) at_cmd: <<< OK
I (7000) at_cmd: >>> AT+SQNMODEACTIVE?
I (7050) at_cmd: <<< OK
// Respuesta: +SQNMODEACTIVE: 1  ✅
```

### Configuración Fallida

```
I (1234) at_cmd: >>> AT+SQNCTM=1
I (6234) at_cmd: <<< ERROR or TIMEOUT  ❌
```

## 🎯 Próximos Pasos

1. Incluye `at_commands.h` en tu `main.cpp`
2. Llama `run_at_diagnostics()` después de inicializar el modem
3. Revisa los logs para ver qué AT commands fallan
4. Usa `configure_rat_with_at_commands()` para forzar NB-IoT
5. Comparte los logs completos para análisis

## 💡 Tips

- Siempre espera suficiente tiempo después de `AT+CFUN=1` (5-10 segundos)
- Usa `AT+SQNMODEACTIVE?` para ver el RAT **realmente activo**
- `AT+SQNCTM?` solo muestra la **configuración**, no el estado actual
- Si nada funciona, prueba factory reset
- Guarda los logs completos para debugging

¡Buena suerte! 🚀
