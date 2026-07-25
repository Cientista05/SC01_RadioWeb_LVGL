#include "stations.h"

const RadioStation stations[] = {
  { 1,
    "Radio Cidade",
    "http://24273.live.streamtheworld.com/RADIOCIDADEAAC.aac" },
  { 2,
    "Antena 1",
    "http://antena1.newradio.it/stream?ext=.mp4" },
  { 3,
    "CBN",
    "http://27423.live.streamtheworld.com/CBN_RJAAC.aac" },
  { 4,
    "Band News FM",
    "http://playerservices.streamtheworld.com/api/livestream-redirect/BANDNEWSFM_SPAAC.aac" },
  { 5,
    "BNR Radio",
    "http://stream.bnr.nl/bnr_mp3_128_20" },
  { 6,
    "Kiss FM",
    "http://24413.live.streamtheworld.com/RADIO_KISSFM_ADP.aac" },
  { 7,
    "89 FM",
    "http://26563.live.streamtheworld.com/RADIO_89FM_ADP.aac" },
  { 8,
    "Jovem Pan",
    "http://cast2.midiazdx.com.br:7110/stream" },
  { 9,
    "Radio Globo",
    "http://playerservices.streamtheworld.com/api/livestream-redirect/RADIO_GLOBO_RJAAC.aac" },
  { 10,
    "JB FM",
    "http://26683.live.streamtheworld.com/JBFMAAC.aac" },
  { 11,
    "KEXP Seattle",
    "http://kexp.streamguys1.com/kexp160.aac" },
  { 12,
    "Radio Swiss Jazz",
    "http://stream.srg-ssr.ch/srgssr/rsj/mp3/128" }
};

const size_t STATION_COUNT = sizeof(stations) / sizeof(stations[0]);

const RadioStation* stationById(uint16_t id) {
  for (size_t i = 0; i < STATION_COUNT; i++) {
    if (stations[i].id == id) {
      return &stations[i];
    }
  }

  return nullptr;
}