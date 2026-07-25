#include "network.h"

#include <WiFi.h>
#include <time.h>

#include "config.h"

// --------------------------------------------------
// CONEXÃO WI-FI
// --------------------------------------------------

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

// --------------------------------------------------
// ESTADO DA REDE
// --------------------------------------------------

bool networkIsConnected() {
  return WiFi.status() == WL_CONNECTED;
}

int32_t networkGetRSSI() {
  if (!networkIsConnected()) {
    return -100;
  }

  return WiFi.RSSI();
}

bool networkGetIPAddress(char* buffer, size_t bufferSize) {
  if (buffer == nullptr || bufferSize == 0) {
    return false;
  }

  if (!networkIsConnected()) {
    strlcpy(buffer, "---", bufferSize);
    return false;
  }

  IPAddress address = WiFi.localIP();

  snprintf(buffer, bufferSize, "%u.%u.%u.%u", address[0], address[1], address[2], address[3]);

  return true;
}

bool networkGetSSID(char* buffer, size_t bufferSize) {
  if (buffer == nullptr || bufferSize == 0) {
    return false;
  }

  if (!networkIsConnected()) {
    strlcpy(buffer, "Offline", bufferSize);

    return false;
  }

  strlcpy(buffer, WiFi.SSID().c_str(), bufferSize);

  return true;
}

// --------------------------------------------------
// DATA E HORÁRIO
// --------------------------------------------------

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
