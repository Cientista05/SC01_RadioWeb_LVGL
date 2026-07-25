#ifndef STORAGE_H
#define STORAGE_H

#include <Arduino.h>

void storageBegin();

size_t storageGetStationIndex();
uint8_t storageGetVolume();

void storageUpdate(size_t stationIndex, uint8_t volume);

#endif