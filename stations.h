#ifndef STATIONS_H
#define STATIONS_H

#include <Arduino.h>

struct RadioStation {
  uint16_t id;       // Identificador único
  const char* name;  // Nome da emissora
  const char* url;   // Stream
};

extern const RadioStation stations[];
extern const size_t STATION_COUNT;

const RadioStation* stationById(uint16_t id);

#endif