#ifndef WEATHER_H
#define WEATHER_H

#include <Arduino.h>

void weatherBegin();
bool weatherGetTemperature(float& temperatureCelsius);

#endif
