#include "weather.h"

#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "network.h"
#include "weather_secrets.h"

// --------------------------------------------------
// ESTADO INTERNO
// --------------------------------------------------

static TaskHandle_t weatherTaskHandle = nullptr;
static portMUX_TYPE weatherMux = portMUX_INITIALIZER_UNLOCKED;

static float currentTemperature = 0.0f;
static bool temperatureAvailable = false;

static constexpr uint32_t WEATHER_RETRY_INTERVAL_MS = 60000UL;
static constexpr uint32_t WEATHER_OFFLINE_INTERVAL_MS = 15000UL;

// --------------------------------------------------
// CONSULTA À OPENWEATHER
// --------------------------------------------------

static bool weatherApiKeyIsConfigured() {
  return OPENWEATHER_API_KEY[0] != '\0' && strcmp(OPENWEATHER_API_KEY, "COLE_SUA_CHAVE_AQUI") != 0;
}

static bool parseTemperature(const String& response, float& temperature) {
  static constexpr const char* TEMPERATURE_FIELD = "\"temp\":";

  int fieldPosition = response.indexOf(TEMPERATURE_FIELD);

  if (fieldPosition < 0) {
    return false;
  }

  const char* valueStart = response.c_str() + fieldPosition + strlen(TEMPERATURE_FIELD);

  char* valueEnd = nullptr;
  float parsedValue = strtof(valueStart, &valueEnd);

  if (valueEnd == valueStart || parsedValue < -90.0f || parsedValue > 60.0f) {
    return false;
  }

  temperature = parsedValue;
  return true;
}

static bool requestTemperature(float& temperature) {
  char url[256];

  snprintf(
    url,
    sizeof(url),
    "https://api.openweathermap.org/data/2.5/weather"
    "?lat=%.4f&lon=%.4f&appid=%s&units=metric",
    WEATHER_LATITUDE,
    WEATHER_LONGITUDE,
    OPENWEATHER_API_KEY);

  WiFiClientSecure secureClient;
  secureClient.setInsecure();

  HTTPClient http;
  http.setConnectTimeout(6000);
  http.setTimeout(6000);

  if (!http.begin(secureClient, url)) {
    Serial.println("[Clima] Falha ao iniciar a consulta");
    return false;
  }

  int responseCode = http.GET();

  if (responseCode != HTTP_CODE_OK) {
    Serial.printf("[Clima] Resposta HTTP: %d\n", responseCode);
    http.end();
    return false;
  }

  String response = http.getString();
  http.end();

  if (!parseTemperature(response, temperature)) {
    Serial.println("[Clima] Temperatura ausente na resposta");
    return false;
  }

  return true;
}

// --------------------------------------------------
// TAREFA DE ATUALIZAÇÃO
// --------------------------------------------------

static void weatherTask(void* parameter) {
  (void)parameter;

  while (true) {
    if (!networkIsConnected()) {
      vTaskDelay(pdMS_TO_TICKS(WEATHER_OFFLINE_INTERVAL_MS));
      continue;
    }

    float temperature = 0.0f;
    bool updated = requestTemperature(temperature);

    if (updated) {
      portENTER_CRITICAL(&weatherMux);
      currentTemperature = temperature;
      temperatureAvailable = true;
      portEXIT_CRITICAL(&weatherMux);

      Serial.printf("[Clima] Temperatura atualizada: %.1f C\n", temperature);
    }

    vTaskDelay(pdMS_TO_TICKS(updated ? WEATHER_UPDATE_INTERVAL_MS : WEATHER_RETRY_INTERVAL_MS));
  }
}

// --------------------------------------------------
// API PÚBLICA
// --------------------------------------------------

void weatherBegin() {
  if (weatherTaskHandle != nullptr) {
    return;
  }

  if (!weatherApiKeyIsConfigured()) {
    Serial.println("[Clima] Configure OPENWEATHER_API_KEY em weather_secrets.h");
    return;
  }

  BaseType_t result = xTaskCreatePinnedToCore(weatherTask, "WeatherTask", 12288, nullptr, 1, &weatherTaskHandle, 1);

  if (result != pdPASS) {
    weatherTaskHandle = nullptr;
    Serial.println("[Clima] Falha ao criar a tarefa");
  }
}

bool weatherGetTemperature(float& temperatureCelsius) {
  portENTER_CRITICAL(&weatherMux);

  bool available = temperatureAvailable;

  if (available) {
    temperatureCelsius = currentTemperature;
  }

  portEXIT_CRITICAL(&weatherMux);

  return available;
}
