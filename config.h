#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include "secrets.h"

// --------------------------------------------------
// ESTAÇÃO
// --------------------------------------------------

constexpr const char* RADIO_URL =
  "http://27613.live.streamtheworld.com/RADIOCIDADEAAC.aac";

constexpr const char* RADIO_NAME = "Rádio Cidade";

// --------------------------------------------------
// ÁUDIO I2S
// --------------------------------------------------

constexpr uint8_t I2S_DOUT = 37;
constexpr uint8_t I2S_BCLK = 36;
constexpr uint8_t I2S_LRC  = 35;

constexpr uint8_t AUDIO_VOLUME = 12;


// --------------------------------------------------
// HORÁRIO
// Brasília
// --------------------------------------------------

constexpr const char* TIMEZONE_INFO = "BRT3";

#endif