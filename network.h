#ifndef NETWORK_H
#define NETWORK_H

#include <Arduino.h>

void networkBegin();
bool networkIsConnected();
int32_t networkGetRSSI();
bool networkGetIPAddress(char* buffer, size_t bufferSize);
bool networkGetSSID(char* buffer, size_t bufferSize);

void networkConfigureTime();
bool networkGetTime(char* buffer, size_t bufferSize);
bool networkGetDate(char* buffer, size_t bufferSize);

#endif
