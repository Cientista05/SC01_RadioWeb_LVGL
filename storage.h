#ifndef STORAGE_H
#define STORAGE_H

#include <Arduino.h>

void storageBegin();

size_t storageGetStationIndex();
uint8_t storageGetVolume();
uint8_t storageGetBrightness();

void storageUpdate(size_t stationIndex, uint8_t volume);
void storageSaveBrightness(uint8_t brightness);

uint8_t storageGetVolumeCloseSeconds();
void storageSaveVolumeCloseSeconds(uint8_t seconds);

#endif
