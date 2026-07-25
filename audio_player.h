#ifndef AUDIO_PLAYER_H
#define AUDIO_PLAYER_H

#include <Arduino.h>

struct AudioMetadata {
  char artist[128];
  char title[160];
  char codec[16];
  char bitrate[20];
};

void audioPlayerBegin();
void audioPlayerStart();

void audioPlayerSetVolume(uint8_t volume);
uint8_t audioPlayerGetVolume();
uint16_t audioPlayerGetVULevel();

bool audioPlayerReadMetadata(AudioMetadata& metadata);
bool audioPlayerHasUpdate();

void audioPlayerSelectStation(size_t index);
size_t audioPlayerGetStationIndex();

void audioPlayerToggle();
bool audioPlayerIsEnabled();

void audioPlayerReconnect();

void audioPlayerSetInitialStation(size_t index);

#endif
