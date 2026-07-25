#ifndef UI_H
#define UI_H

#include <Arduino.h>

void uiBegin();
void uiUpdate();

void uiShowStatus(const char* message);
void uiClearStatus();

void uiSetBrightness(uint8_t brightness);

void uiSetVolumeCloseSeconds(uint8_t seconds);

#endif
