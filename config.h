#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include "secrets.h"

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

// Versão exibida no painel Sistema
constexpr const char* FIRMWARE_VERSION = "1.0.1";

#endif
