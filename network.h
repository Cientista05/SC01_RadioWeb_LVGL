#ifndef NETWORK_H
#define NETWORK_H

#include <Arduino.h>

void networkBegin();
bool networkIsConnected();
int32_t networkGetRSSI();

void networkConfigureTime();
bool networkGetTime(char* buffer, size_t bufferSize);
bool networkGetDate(char* buffer, size_t bufferSize);

#endif