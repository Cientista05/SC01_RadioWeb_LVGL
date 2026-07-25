#ifndef UI_H
#define UI_H

#include <Arduino.h>

void uiBegin();
void uiUpdate();

void uiShowStatus(const char* message);
void uiClearStatus();

bool uiIsStationListOpen();

#endif