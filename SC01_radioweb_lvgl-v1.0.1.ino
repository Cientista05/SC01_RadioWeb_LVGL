#include <Arduino.h>
#include "storage.h"
#include "network.h"
#include "audio_player.h"
#include "ui.h"

static bool audioStarted = false;
static bool timeConfigured = false;
static bool lastNetworkConnected = false;

void setup() {
  Serial.begin(115200);

  // Interface continua funcionando mesmo sem Wi-Fi
  uiBegin();

  storageBegin();
  uiSetBrightness(storageGetBrightness());
  uiSetVolumeCloseSeconds(storageGetVolumeCloseSeconds());

  audioPlayerSetInitialStation(storageGetStationIndex());
  audioPlayerSetVolume(storageGetVolume());

  // Prepara o áudio, mas ainda não inicia o stream
  audioPlayerBegin();

  // Inicia a conexão sem bloquear o setup
  networkBegin();
}

void loop() {
  uiUpdate();

  storageUpdate(audioPlayerGetStationIndex(), audioPlayerGetVolume());
  bool connected = networkIsConnected();

  // Estado da rede mudou
  if (connected != lastNetworkConnected) {
    lastNetworkConnected = connected;

    if (connected) {
      Serial.println();
      Serial.println("[WiFi] Conectado");

      uiClearStatus();

      if (!timeConfigured) {
        networkConfigureTime();
        timeConfigured = true;
      }

      if (!audioStarted) {
        audioPlayerStart();
        audioStarted = true;
      } else {
        // O Wi-Fi caiu e retornou
        audioPlayerReconnect();
      }
    } else {
      Serial.println();
      Serial.println("[WiFi] Desconectado");

      uiShowStatus("Sem Wi-Fi");
    }
  }

  delay(5);
}
