#include "storage.h"

#include <Preferences.h>

#include "config.h"

// --------------------------------------------------
// ESTADO INTERNO
// --------------------------------------------------

static Preferences preferences;

static bool storageReady = false;
static bool settingsDirty = false;

static uint16_t savedStation = 0;
static uint8_t savedVolume = AUDIO_VOLUME;

static uint16_t pendingStation = 0;
static uint8_t pendingVolume = AUDIO_VOLUME;

static uint32_t lastSettingsChange = 0;

static constexpr uint32_t SAVE_DELAY = 2000;

static uint8_t savedBrightness = 70;

static uint8_t savedVolumeCloseSeconds = 3;

// --------------------------------------------------
// INICIALIZAÇÃO E LEITURA
// --------------------------------------------------

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

  savedBrightness =
    preferences.getUChar("brightness", 70);

  if (
    savedBrightness < 10 || savedBrightness > 100) {
    savedBrightness = 70;
  }

  Serial.printf(
    "[Storage] Estacao: %u, volume: %u, brilho: %u%%\n",
    savedStation,
    savedVolume,
    savedBrightness);

  savedVolumeCloseSeconds =
    preferences.getUChar("vol-close", 3);

  if (
    savedVolumeCloseSeconds != 3 && savedVolumeCloseSeconds != 5 && savedVolumeCloseSeconds != 10) {
    savedVolumeCloseSeconds = 3;
  }
}

size_t storageGetStationIndex() {
  return savedStation;
}

uint8_t storageGetVolume() {
  return savedVolume;
}

uint8_t storageGetBrightness() {
  return savedBrightness;
}

uint8_t storageGetVolumeCloseSeconds() {
  return savedVolumeCloseSeconds;
}

// --------------------------------------------------
// GRAVAÇÃO ADIADA: ESTAÇÃO E VOLUME
// --------------------------------------------------

void storageUpdate(size_t stationIndex, uint8_t volume) {

  if (!storageReady) {
    return;
  }

  if (volume > 21) {
    volume = 21;
  }

  uint16_t station = static_cast<uint16_t>(stationIndex);

  if (station != pendingStation || volume != pendingVolume) {
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

// --------------------------------------------------
// GRAVAÇÃO IMEDIATA: CONFIGURAÇÕES DA INTERFACE
// --------------------------------------------------

void storageSaveBrightness(uint8_t brightness) {
  if (!storageReady) {
    return;
  }

  if (brightness < 10) {
    brightness = 10;
  }

  if (brightness > 100) {
    brightness = 100;
  }

  if (brightness == savedBrightness) {
    return;
  }

  if (preferences.putUChar("brightness", brightness) == 0) {
    Serial.println("[Storage] Falha ao salvar brilho");
    return;
  }

  savedBrightness = brightness;

  Serial.printf("[Storage] Brilho salvo: %u%%\n", savedBrightness);
}

void storageSaveVolumeCloseSeconds(uint8_t seconds) {
  if (!storageReady) {
    return;
  }

  if (
    seconds != 3 && seconds != 5 && seconds != 10) {
    return;
  }

  if (seconds == savedVolumeCloseSeconds) {
    return;
  }

  if (preferences.putUChar("vol-close", seconds) == 0) {
    Serial.println("[Storage] Falha ao salvar tempo do volume");
    return;
  }

  savedVolumeCloseSeconds = seconds;

  Serial.printf("[Storage] Tempo do volume salvo: %us\n", savedVolumeCloseSeconds);
}
