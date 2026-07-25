#include "network.h"

#include <WiFi.h>
#include <time.h>

#include "config.h"

void networkBegin() {
  WiFi.mode(WIFI_STA);

  // Não grava repetidamente na flash
  WiFi.persistent(false);

  // Reconexão automática
  WiFi.setAutoReconnect(true);

  // Ajuda a manter o streaming estável
  WiFi.setSleep(false);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

bool networkIsConnected() {
  return WiFi.status() == WL_CONNECTED;
}

int32_t networkGetRSSI() {
  if (!networkIsConnected()) {
    return -100;
  }

  return WiFi.RSSI();
}

void networkConfigureTime() {
  configTzTime(TIMEZONE_INFO, "pool.ntp.org", "time.google.com", "time.cloudflare.com");
}

bool networkGetTime(char* buffer, size_t bufferSize) {
  if (buffer == nullptr || bufferSize == 0) {
    return false;
  }

  struct tm timeInfo;

  if (!getLocalTime(&timeInfo, 20)) {
    strlcpy(buffer, "--:--", bufferSize);
    return false;
  }

  strftime(buffer, bufferSize, "%H:%M", &timeInfo);

  return true;
}

bool networkGetDate(char* buffer, size_t bufferSize) {
  if (buffer == nullptr || bufferSize == 0) {
    return false;
  }

  struct tm timeInfo;

  if (!getLocalTime(&timeInfo, 20)) {
    strlcpy(buffer, "-- --- ----", bufferSize);
    return false;
  }

  static const char* meses[] = {
    "Jan", "Fev", "Mar", "Abr",
    "Mai", "Jun", "Jul", "Ago",
    "Set", "Out", "Nov", "Dez"
  };

  snprintf(buffer, bufferSize, "%02d %s %04d", timeInfo.tm_mday, meses[timeInfo.tm_mon], timeInfo.tm_year + 1900);

  return true;
}