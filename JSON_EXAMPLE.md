# Ejemplo de Transmisión JSON con Walter NB-IoT

Este proyecto incluye ejemplos completos de cómo enviar datos JSON a través de HTTP usando el modem Walter.

## 📋 Características

- ✅ Envío de JSON via HTTP POST
- ✅ Creación automática de objetos JSON
- ✅ Ejemplos con datos de sensores
- ✅ Transmisión periódica automática
- ✅ Servidor de prueba incluido (httpbin.org)

## 🚀 Uso Rápido

### 1. El código ya está configurado

Después de conectarse a NB-IoT, el código automáticamente:
1. Envía 2 ejemplos de JSON a httpbin.org
2. Luego envía datos cada 60 segundos

### 2. Ejemplos Incluidos

#### Ejemplo 1: Telemetría Simple
```json
{
  "device": "walter-test",
  "temp": 25.3,
  "hum": 60.5,
  "time": 12345
}
```

#### Ejemplo 2: Datos de Sensores Completos
```json
{
  "device_id": "walter-001",
  "device_type": "nbiot-sensor",
  "timestamp": 12345,
  "sensors": {
    "temperature": 23.5,
    "humidity": 65.2,
    "pressure": 1013.25
  },
  "location": {
    "latitude": 40.7128,
    "longitude": -74.0060
  },
  "status": "online",
  "battery_level": 85
}
```

## 🔧 Personalización

### Cambiar la URL del Servidor

En `main.cpp`, línea 491:
```cpp
send_json_http("http://tu-servidor.com/api/data", json);
```

### Cambiar los Datos del JSON

Opción 1 - Usar la función helper:
```cpp
char* json = create_custom_json("mi-dispositivo", 26.5, 70.0);
```

Opción 2 - Crear tu propio JSON:
```cpp
cJSON *root = cJSON_CreateObject();
cJSON_AddStringToObject(root, "device", "walter-001");
cJSON_AddNumberToObject(root, "temperatura", 25.5);
cJSON_AddNumberToObject(root, "humedad", 65.0);

// Agregar array
cJSON *array = cJSON_CreateArray();
cJSON_AddItemToArray(array, cJSON_CreateNumber(1));
cJSON_AddItemToArray(array, cJSON_CreateNumber(2));
cJSON_AddItemToObject(root, "valores", array);

char *json_string = cJSON_PrintUnformatted(root);
cJSON_Delete(root);

// Usar json_string...
cJSON_free(json_string);
```

### Cambiar la Frecuencia de Envío

En `main.cpp`, línea 484:
```cpp
vTaskDelay(pdMS_TO_TICKS(60000)); // 60 segundos = 60000 ms
```

Ejemplos:
- Cada 5 minutos: `300000`
- Cada 10 minutos: `600000`
- Cada hora: `3600000`

## 📡 Funciones Disponibles

### `send_json_http(url, json_data)`
Envía JSON via HTTP POST.

**Parámetros:**
- `url`: URL del servidor (debe empezar con http://)
- `json_data`: String JSON a enviar

**Retorna:** `true` si exitoso, `false` si falla

**Ejemplo:**
```cpp
char* json = create_custom_json("walter-001", 25.0, 60.0);
if (send_json_http("http://mi-servidor.com/api", json)) {
    ESP_LOGI(TAG, "Enviado!");
}
cJSON_free(json);
```

### `create_sensor_json()`
Crea un JSON completo con datos de sensores.

**Retorna:** String JSON (debes liberar con `cJSON_free()`)

**Ejemplo:**
```cpp
char* json = create_sensor_json();
send_json_http("http://servidor.com/api", json);
cJSON_free(json);
```

### `create_custom_json(device_id, temperature, humidity)`
Crea un JSON simple personalizado.

**Parámetros:**
- `device_id`: ID del dispositivo
- `temperature`: Temperatura
- `humidity`: Humedad

**Ejemplo:**
```cpp
char* json = create_custom_json("sensor-01", 23.5, 65.0);
```

## 🌐 Servidores de Prueba

### httpbin.org (Incluido)
```
http://httpbin.org/post
```
- Gratis
- Devuelve el JSON que enviaste
- Perfecto para testing

### RequestBin
```
https://requestbin.com
```
- Crea un endpoint temporal
- Ve todos los requests en tiempo real

### Webhook.site
```
https://webhook.site
```
- Endpoint único temporal
- Interfaz web para ver requests

## 🔐 HTTPS (SSL/TLS)

Para usar HTTPS, modifica en `http_json_example.h`:

```cpp
if (!modem.httpConfigProfile(
    "application/json",
    NULL,
    NULL,
    NULL,
    true,  // ← Cambiar a true para HTTPS
    &rsp)) {
```

**Nota:** HTTPS requiere más memoria y tiempo de procesamiento.

## 📊 Ejemplo con Tu Propio Servidor

### Backend Node.js (Express)
```javascript
const express = require('express');
const app = express();

app.use(express.json());

app.post('/api/sensor', (req, res) => {
    console.log('Datos recibidos:', req.body);
    res.json({ status: 'ok', received: true });
});

app.listen(3000);
```

### Backend Python (Flask)
```python
from flask import Flask, request, jsonify

app = Flask(__name__)

@app.route('/api/sensor', methods=['POST'])
def sensor_data():
    data = request.json
    print('Datos recibidos:', data)
    return jsonify({'status': 'ok', 'received': True})

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=3000)
```

### Código ESP32
```cpp
send_json_http("http://tu-servidor.com:3000/api/sensor", json);
```

## 🐛 Troubleshooting

### Error: "Failed to configure HTTP profile"
- Verifica que estés conectado a la red
- Asegúrate que el PDP context esté activo

### Error: "HTTP request failed with status: 404"
- Verifica la URL
- Asegúrate que el servidor esté accesible

### Error: "Failed to send HTTP POST request"
- Verifica la conexión de red
- Revisa que el JSON sea válido
- Verifica que el servidor acepte POST

### El JSON no llega al servidor
- Verifica que el servidor esté en internet público (no localhost)
- Usa httpbin.org primero para confirmar que funciona
- Revisa los logs del servidor

## 💡 Tips

1. **Usa httpbin.org primero** - Confirma que todo funciona antes de usar tu servidor
2. **JSON compacto** - Usa `cJSON_PrintUnformatted()` en lugar de `cJSON_Print()` para ahorrar datos
3. **Manejo de errores** - Siempre verifica el retorno de `send_json_http()`
4. **Libera memoria** - Siempre llama `cJSON_free()` después de usar el JSON
5. **Batch sending** - Agrupa múltiples lecturas en un solo JSON para ahorrar datos

## 📈 Optimización de Datos

NB-IoT cobra por datos transmitidos. Optimiza:

### JSON Compacto
```cpp
// ❌ Mal (verboso)
{"temperature": 25.5, "humidity": 60.0}

// ✅ Mejor (compacto)
{"t": 25.5, "h": 60.0}
```

### Batch de Datos
```cpp
cJSON *root = cJSON_CreateObject();
cJSON *readings = cJSON_CreateArray();

// Agregar múltiples lecturas
for (int i = 0; i < 10; i++) {
    cJSON *reading = cJSON_CreateObject();
    cJSON_AddNumberToObject(reading, "t", get_temperature());
    cJSON_AddItemToArray(readings, reading);
}

cJSON_AddItemToObject(root, "data", readings);
```

## 🔄 Integración con Plataformas IoT

### ThingSpeak
```cpp
// URL: https://api.thingspeak.com/update
// JSON: {"api_key":"TU_API_KEY","field1":25.5}
```

### Ubidots
```cpp
// URL: http://industrial.api.ubidots.com/api/v1.6/devices/walter
// Header: X-Auth-Token: TU_TOKEN
```

### AWS IoT Core
Requiere MQTT (ver ejemplos de MQTT en la librería Walter)

## 📝 Logs de Ejemplo

```
I (12345) walter_nbiot: CONNECTION SUCCESSFUL!
I (12347) walter_nbiot: Testing JSON Transmission
I (12350) http_json: Sending JSON to: http://httpbin.org/post
I (12355) http_json: JSON data: {"device":"walter-test","temp":25.3,"hum":60.5,"time":12345}
I (15234) http_json: HTTP Status: 200
I (15235) http_json: Response length: 523 bytes
I (15236) walter_nbiot: ✓ Telemetry sent successfully!
```

## 🎯 Próximos Pasos

1. ✅ Prueba con httpbin.org
2. ✅ Modifica el JSON con tus datos
3. ✅ Configura tu propio servidor
4. ✅ Ajusta la frecuencia de envío
5. ✅ Implementa tu lógica de negocio

¡Listo para transmitir datos! 🚀
