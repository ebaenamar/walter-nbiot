# Debug Guide - Walter NB-IoT

Esta guía te ayudará a debuggear problemas de conexión con tu placa Walter.

## Modo Debug

El código ahora incluye un **modo debug completo** que muestra información detallada sobre:

- Comandos AT enviados al modem
- Estado operacional del modem
- Configuración RAT (Radio Access Technology)
- Calidad de señal
- Estado de la SIM
- Información de red
- Bandas de radio configuradas

### Activar/Desactivar Debug

En `main/main.cpp`, línea 25:

```cpp
#define DEBUG_MODE true   // true = modo debug ON, false = modo normal
```

## Información de Debug

### 1. Diagnóstico Completo del Modem

Al inicio (después de establecer comunicación), verás:

```
I (xxx) walter_debug: ========================================
I (xxx) walter_debug: Running Modem Diagnostics
I (xxx) walter_debug: ========================================
I (xxx) walter_debug: Sending: ATI (Modem identification)
I (xxx) walter_debug: Sending: AT+CGMR (Firmware version)
I (xxx) walter_debug: Sending: AT+CGSN (IMEI)
I (xxx) walter_debug: Sending: AT+CIMI (IMSI)
I (xxx) walter_debug: Sending: AT+CCID (SIM ICCID)
I (xxx) walter_debug: Sending: AT+CPIN? (SIM PIN status)
I (xxx) walter_debug: Sending: AT+COPS? (Current operator)
I (xxx) walter_debug: Sending: AT+CEREG? (Network registration status)
I (xxx) walter_debug: Sending: AT+CSQ (Signal quality)
I (xxx) walter_debug: Sending: AT+URAT? (Current RAT setting)
I (xxx) walter_debug: Sending: AT+UBANDMASK? (Band mask)
I (xxx) walter_debug: Sending: AT+CFUN? (Functionality level)
I (xxx) walter_debug: Sending: AT+CGDCONT? (PDP context definition)
I (xxx) walter_debug: Sending: AT+CGACT? (PDP context activation state)
I (xxx) walter_debug: Sending: AT+CGATT? (GPRS attachment state)
```

### 2. Verificación de Soporte RAT

Antes de intentar configurar el RAT, verás:

```
I (xxx) walter_debug: Checking RAT support:
I (xxx) walter_debug: Sending: AT+URAT=? (Supported RAT values)
I (xxx) walter_debug: Current RAT: 1 (NB-IoT)
```

**Valores RAT:**
- `0` = LTE-M (CAT-M1)
- `1` = NB-IoT
- `2` = GSM

### 3. Configuración Detallada de RAT

Al configurar el RAT:

```
I (xxx) walter_debug: Attempting to set RAT to NB-IoT (1)
I (xxx) walter_debug:   Current op state: 0
I (xxx) walter_debug:   RAT set successfully
I (xxx) walter_debug:   Verified RAT: 1
```

O si falla:

```
E (xxx) walter_debug:   RAT set FAILED (result: -1)
I (xxx) walter_debug:   Verified RAT: 0
```

### 4. Verificación de Cobertura de Red

Antes de intentar registrarse:

```
I (xxx) walter_debug: Checking network coverage:
I (xxx) walter_debug:   RSSI: -75 dBm
I (xxx) walter_debug:   RSRP: -95 dBm
I (xxx) walter_debug:   RSRQ: -12 dB
I (xxx) walter_debug:   SNR: 5 dB
I (xxx) walter_debug:   Signal: GOOD
I (xxx) walter_debug:   Registration: Searching (2)
```

**Interpretación de Señal (RSRP):**
- `> -80 dBm` = EXCELLENT
- `-80 a -90 dBm` = GOOD
- `-90 a -100 dBm` = FAIR
- `-100 a -110 dBm` = POOR
- `< -110 dBm` = VERY POOR

### 5. Estados de Registro de Red

```
0 = Not searching (No está buscando)
1 = Registered (Home) (Registrado en red local)
2 = Searching (Buscando red)
3 = Registration denied (Registro denegado)
5 = Registered (Roaming) (Registrado en roaming)
```

## Problemas Comunes y Soluciones

### Problema: "Failed to set RAT to NB-IoT"

**Diagnóstico:**
1. Verifica el estado operacional antes de cambiar RAT
2. Debe estar en `MINIMUM` (estado 0)
3. Revisa si el modem soporta NB-IoT con `AT+URAT=?`

**Solución:**
```
El código ahora:
1. Cambia a estado MINIMUM
2. Espera 2 segundos
3. Configura RAT
4. Verifica que se aplicó
5. Vuelve a estado FULL
```

### Problema: "Network registration timeout"

**Diagnóstico en modo debug:**

Verás información como:
```
E (xxx) walter_nbiot: Network registration failed - gathering diagnostic info:
I (xxx) walter_debug: Checking network coverage:
I (xxx) walter_debug:   RSSI: -115 dBm
I (xxx) walter_debug:   RSRP: -125 dBm
I (xxx) walter_debug:   Signal: VERY POOR
```

**Causas posibles:**

1. **Señal muy débil** (RSRP < -110 dBm)
   - Solución: Muévete a un lugar con mejor señal
   - Verifica la antena está conectada

2. **RAT incorrecto**
   ```
   E (xxx) walter_debug:   Current RAT: 2
   ```
   - Si muestra GSM (2) pero necesitas NB-IoT/LTE-M
   - Verifica que tu operador soporta la tecnología

3. **SIM no activada**
   ```
   I (xxx) walter_debug: Sending: AT+CPIN? (SIM PIN status)
   E (xxx) walter_debug:   Response FAILED
   ```
   - Verifica en consola Soracom que la SIM está activa

4. **Sin cobertura NB-IoT/LTE-M**
   - Verifica cobertura en https://www.soracom.io/coverage/
   - Algunas áreas solo tienen cobertura 2G/3G

### Problema: "Cannot communicate with modem"

**Diagnóstico:**
```
E (xxx) walter_nbiot: Cannot communicate with modem
```

**Soluciones:**
1. Verifica conexión USB
2. Prueba cambiar UART:
   ```cpp
   #define MODEM_UART_NUM UART_NUM_0  // o UART_NUM_2
   ```
3. Verifica alimentación adecuada
4. Resetea la placa

## Comandos AT Útiles

Si necesitas enviar comandos AT manualmente, puedes usar:

```cpp
send_debug_command("AT+COMANDO", "Descripción");
```

**Comandos útiles:**

```
AT+COPS?        // Ver operador actual
AT+CEREG?       // Estado de registro
AT+CSQ          // Calidad de señal
AT+URAT?        // RAT actual
AT+URAT=?       // RATs soportados
AT+CGDCONT?     // Contextos PDP definidos
AT+CGACT?       // Estado de activación PDP
AT+CFUN?        // Nivel de funcionalidad
AT+CPIN?        // Estado del PIN
```

## Logs Completos

Para obtener logs completos para análisis:

```bash
idf.py monitor > walter_debug.log 2>&1
```

Esto guardará todo el output en `walter_debug.log`.

## Información para Soporte

Si necesitas ayuda, incluye:

1. **Versión del firmware del modem**
   - Se muestra en el diagnóstico inicial
   
2. **RAT configurado vs RAT soportado**
   - Del output de `check_rat_support()`

3. **Calidad de señal**
   - RSSI, RSRP, RSRQ, SNR

4. **Estado de registro**
   - Valor numérico y descripción

5. **Ubicación aproximada**
   - Para verificar cobertura

6. **Logs completos**
   - Desde el inicio hasta el error

## Desactivar Debug Mode

Para producción, desactiva el modo debug:

```cpp
#define DEBUG_MODE false
```

Esto reducirá:
- Output en consola
- Tiempo de inicio
- Uso de memoria

## Próximos Pasos

Una vez que identifiques el problema:

1. Revisa `TROUBLESHOOTING.md` para soluciones específicas
2. Ajusta configuración según diagnóstico
3. Si el problema persiste, contacta soporte con logs completos
