#ifndef DISPLAY_CONFIG_H
#define DISPLAY_CONFIG_H

#include <Arduino.h>
#include <LovyanGFX.hpp>

#include "ST7796U.h"

constexpr uint8_t DISPLAY_ROTATION = 3;
constexpr uint8_t DISPLAY_BRIGHTNESS = 180;

#define COLOR_BACKGROUND TFT_BLUE
#define COLOR_TEXT TFT_WHITE
#define COLOR_SECONDARY TFT_CYAN
#define COLOR_RSSI_ON TFT_GREEN
#define COLOR_RSSI_OFF 0x420800

#endif