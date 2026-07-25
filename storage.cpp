#include "storage.h"

#include <Preferences.h>

#include "config.h"

static Preferences preferences;

static bool storageReady = false;
static bool settingsDirty = false;

static uint16_t savedStation = 0;
static uint8_t savedVolume = AUDIO_VOLUME;

static uint16_t pendingStation = 0;
static uint8_t pendingVolume = AUDIO_VOLUME;

static uint32_t lastSettingsChange = 0;

static constexpr uint32_t SAVE_DELAY = 2000;

void storageBegin() {
  storageReady = preferences.begin("web-radio", false);

  if (!storageReady) {
    Serial.println("[Storage] Falha ao iniciar");
    return;
  }

  savedStation = preferences.getUShort("station", 0);

  savedVolume = preferences.getUChar("volume", AUDIO_VOLUME);

  if (savedVolume > 21) {
    savedVolume = AUDIO_VOLUME;
  }

  pendingStation = savedStation;
  pendingVolume = savedVolume;

  Serial.printf("[Storage] Estacao: %u, volume: %u\n", savedStation, savedVolume);
}

size_t storageGetStationIndex() {
  return savedStation;
}

uint8_t storageGetVolume() {
  return savedVolume;
}

void storageUpdate(size_t stationIndex, uint8_t volume) {

  if (!storageReady) {
    return;
  }

  if (volume > 21) {
    volume = 21;
  }

  uint16_t station = static_cast<uint16_t>(stationIndex);

  if (
    station != pendingStation || volume != pendingVolume) {

    pendingStation = station;
    pendingVolume = volume;

    lastSettingsChange = millis();
    settingsDirty = true;
  }

  if (!settingsDirty || millis() - lastSettingsChange < SAVE_DELAY) {
    return;
  }

  if (pendingStation != savedStation) {
    preferences.putUShort("station", pendingStation);

    savedStation = pendingStation;
  }

  if (pendingVolume != savedVolume) {
    preferences.putUChar("volume", pendingVolume);

    savedVolume = pendingVolume;
  }

  settingsDirty = false;

  Serial.printf("[Storage] Salvo - estacao: %u, volume: %u\n", savedStation, savedVolume);
}