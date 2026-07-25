#include "ui.h"

#include <lvgl.h>
#include "stations.h"
#include "display_config.h"
#include "network.h"
#include "audio_player.h"

// --------------------------------------------------
// DISPLAY E BUFFER
// --------------------------------------------------

static LGFX lcd;

static constexpr uint16_t SCREEN_WIDTH = 480;
static constexpr uint16_t SCREEN_HEIGHT = 320;
static constexpr uint16_t BUFFER_LINES = 10;

static lv_disp_draw_buf_t drawBuffer;
static lv_color_t bufferMemory[SCREEN_WIDTH * BUFFER_LINES];

static lv_disp_drv_t displayDriver;
static lv_indev_drv_t touchDriver;

static uint32_t lastLvglTick = 0;
static uint32_t lastHeaderUpdate = 0;

// --------------------------------------------------
// OBJETOS DA INTERFACE
// --------------------------------------------------

static lv_obj_t* rssiBars[4] = {
  nullptr, nullptr, nullptr, nullptr
};

static lv_obj_t* rssiLabel = nullptr;
static lv_obj_t* clockLabel = nullptr;
static lv_obj_t* dateLabel = nullptr;
static lv_obj_t* codecLabel = nullptr;
static lv_obj_t* bitrateLabel = nullptr;
static lv_obj_t* statusLabel = nullptr;

static lv_obj_t* artistLabel = nullptr;
static lv_obj_t* titleLabel = nullptr;

static lv_anim_t scrollAnimation;
static lv_style_t scrollStyle;

static char lastArtist[128] = "";
static char lastTitle[160] = "";

static constexpr size_t MAX_STATION_BUTTONS = 32;

static lv_obj_t* stationPanel = nullptr;
static lv_obj_t* stationList = nullptr;

static lv_obj_t* stationButtons[MAX_STATION_BUTTONS] = {};

static bool stationListOpen = false;

static bool touchWasPressed = false;
static uint16_t touchStartY = 0;
static uint16_t touchStartX = 0;

static lv_obj_t* volumePanel = nullptr;
static lv_obj_t* volumeTitleLabel = nullptr;
static lv_obj_t* volumeValueLabel = nullptr;
static lv_obj_t* volumePercentLabel = nullptr;
static lv_obj_t* volumeControlArea = nullptr;

static lv_obj_t* volumeSegments[21] = {};

static bool volumePanelOpen = false;

static uint32_t volumeLastInteraction = 0;

static int16_t volumeLastX = 0;
static int16_t volumeMovementAccumulator = 0;

static constexpr uint32_t VOLUME_CLOSE_TIME = 3000;
static constexpr int16_t PIXELS_PER_VOLUME_STEP = 8;

// --------------------------------------------------
// CALLBACK DO DISPLAY
// --------------------------------------------------

static void displayFlush(lv_disp_drv_t* driver, const lv_area_t* area, lv_color_t* colorData) {

  uint32_t width = area->x2 - area->x1 + 1;
  uint32_t height = area->y2 - area->y1 + 1;

  lcd.startWrite();
  lcd.setAddrWindow(area->x1, area->y1, width, height);
  lcd.writePixels(reinterpret_cast<lgfx::rgb565_t*>(&colorData->full), width * height);
  lcd.endWrite();

  lv_disp_flush_ready(driver);
}

// --------------------------------------------------
// CALLBACK DO TOUCH
// --------------------------------------------------

static void touchRead(lv_indev_drv_t* driver, lv_indev_data_t* data) {

  (void)driver;

  uint16_t x = 0;
  uint16_t y = 0;

  bool pressed = lcd.getTouch(&x, &y);

  if (pressed) {
    if (!touchWasPressed) {
      touchWasPressed = true;
      touchStartX = x;
      touchStartY = y;
    }

    data->state = LV_INDEV_STATE_PR;
    data->point.x = x;
    data->point.y = y;
  } else {
    touchWasPressed = false;
    data->state = LV_INDEV_STATE_REL;
  }
}

// --------------------------------------------------
// PLAY / STOP INVISÍVEL
// --------------------------------------------------

static void playStopEvent(lv_event_t* event) {
  if (lv_event_get_code(event) != LV_EVENT_CLICKED) {
    return;
  }

  Serial.println("[Touch] Play/Stop");
  audioPlayerToggle();
}

// --------------------------------------------------
// CRIAÇÃO DO CABEÇALHO
// --------------------------------------------------

static void createHeader() {
  // Linha inferior
  lv_obj_t* divider = lv_obj_create(lv_scr_act());

  lv_obj_set_size(divider, 460, 2);
  lv_obj_set_pos(divider, 10, 58);

  lv_obj_set_style_bg_color(divider, lv_color_hex(0x00FFFF), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(divider, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_border_width(divider, 0, LV_PART_MAIN);
  lv_obj_clear_flag(divider, LV_OBJ_FLAG_SCROLLABLE);

  // Barras de Wi-Fi
  for (uint8_t i = 0; i < 4; i++) {
    int16_t height = 8 + (i * 7);

    rssiBars[i] = lv_obj_create(lv_scr_act());

    lv_obj_set_size(rssiBars[i], 7, height);
    lv_obj_set_pos(rssiBars[i], 12 + (i * 11), 43 - height);
    lv_obj_set_style_radius(rssiBars[i], 2, LV_PART_MAIN);
    lv_obj_set_style_border_width(rssiBars[i], 0, LV_PART_MAIN);
    lv_obj_clear_flag(rssiBars[i], LV_OBJ_FLAG_SCROLLABLE);
  }

  rssiLabel = lv_label_create(lv_scr_act());

  lv_obj_set_style_text_color(rssiLabel, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_text_font(rssiLabel, &lv_font_montserrat_14, LV_PART_MAIN);
  lv_obj_set_pos(rssiLabel, 60, 20);

  // Relógio
  clockLabel = lv_label_create(lv_scr_act());

  lv_obj_set_style_text_color(clockLabel, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_text_font(clockLabel, &lv_font_montserrat_28, LV_PART_MAIN);
  lv_obj_align(clockLabel, LV_ALIGN_TOP_MID, 0, 1);

  // Data
  dateLabel = lv_label_create(lv_scr_act());

  lv_obj_set_style_text_color(dateLabel, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_text_font(dateLabel, &lv_font_montserrat_14, LV_PART_MAIN);
  lv_obj_align(dateLabel, LV_ALIGN_TOP_MID, 0, 38);

  // Codec
  codecLabel = lv_label_create(lv_scr_act());

  lv_obj_set_style_text_color(codecLabel, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_text_font(codecLabel, &lv_font_montserrat_18, LV_PART_MAIN);
  lv_obj_align(codecLabel, LV_ALIGN_TOP_RIGHT, -10, 5);

  // Bitrate
  bitrateLabel = lv_label_create(lv_scr_act());

  lv_obj_set_style_text_color(bitrateLabel, lv_color_hex(0x44FF66), LV_PART_MAIN);
  lv_obj_set_style_text_font(bitrateLabel, &lv_font_montserrat_14, LV_PART_MAIN);
  lv_obj_align(bitrateLabel, LV_ALIGN_TOP_RIGHT, -10, 34);
}

// --------------------------------------------------
// ATUALIZAÇÃO DO CABEÇALHO
// --------------------------------------------------

static void updateHeader() {
  int32_t rssi = networkGetRSSI();
  uint8_t activeBars = 0;

  if (networkIsConnected()) {
    if (rssi >= -55) {
      activeBars = 4;
    } else if (rssi >= -67) {
      activeBars = 3;
    } else if (rssi >= -75) {
      activeBars = 2;
    } else if (rssi >= -85) {
      activeBars = 1;
    }
  }

  for (uint8_t i = 0; i < 4; i++) {
    lv_obj_set_style_bg_color(
      rssiBars[i],
      i < activeBars
        ? lv_color_hex(0x32E875)
        : lv_color_hex(0x405A70),
      LV_PART_MAIN);
  }

  char rssiText[20];

  if (networkIsConnected()) {
    snprintf(rssiText, sizeof(rssiText), "%ld dBm", static_cast<long>(rssi));
  } else {
    strlcpy(rssiText, "Offline", sizeof(rssiText));
  }

  lv_label_set_text(rssiLabel, rssiText);

  char currentClock[8];
  char currentDate[16];

  networkGetTime(currentClock, sizeof(currentClock));
  networkGetDate(currentDate, sizeof(currentDate));

  lv_label_set_text(clockLabel, currentClock);
  lv_label_set_text(dateLabel, currentDate);

  AudioMetadata metadata;
  audioPlayerReadMetadata(metadata);

  lv_label_set_text(codecLabel, metadata.codec[0] != '\0' ? metadata.codec : "---");
  lv_label_set_text(bitrateLabel, metadata.bitrate[0] != '\0' ? metadata.bitrate : "--- kbps");
}

// --------------------------------------------------
// ARTISTA E MÚSICA
// --------------------------------------------------

static void createMetadata() {
  // Pausa antes de iniciar e repetir a rolagem
  lv_anim_init(&scrollAnimation);
  lv_anim_set_delay(&scrollAnimation, 1800);
  lv_anim_set_repeat_delay(&scrollAnimation, 1800);
  lv_style_init(&scrollStyle);
  lv_style_set_anim(&scrollStyle, &scrollAnimation);

  // Artista
  artistLabel = lv_label_create(lv_scr_act());

  lv_label_set_long_mode(artistLabel, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_obj_set_size(artistLabel, 450, 45);
  lv_obj_set_pos(artistLabel, 15, 105);
  lv_obj_set_style_text_font(artistLabel, &lv_font_montserrat_24, LV_PART_MAIN);
  lv_obj_set_style_text_color(artistLabel, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_text_align(artistLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_add_style(artistLabel, &scrollStyle, LV_STATE_DEFAULT);

  // Linha entre artista e música
  lv_obj_t* divider = lv_obj_create(lv_scr_act());

  lv_obj_set_size(divider, 300, 2);
  lv_obj_set_pos(divider, 90, 177);
  lv_obj_set_style_bg_color(divider, lv_color_hex(0x00FFFF), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(divider, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_border_width(divider, 0, LV_PART_MAIN);
  lv_obj_clear_flag(divider, LV_OBJ_FLAG_SCROLLABLE);

  // Música
  titleLabel = lv_label_create(lv_scr_act());

  lv_label_set_long_mode(titleLabel, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_obj_set_size(titleLabel, 450, 40);
  lv_obj_set_pos(titleLabel, 15, 195);
  lv_obj_set_style_text_font(titleLabel, &lv_font_montserrat_20, LV_PART_MAIN);
  lv_obj_set_style_text_color(titleLabel, lv_color_hex(0x00FFFF), LV_PART_MAIN);
  lv_obj_set_style_text_align(titleLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_add_style(titleLabel, &scrollStyle, LV_STATE_DEFAULT);
}

static void updateMetadata() {
  AudioMetadata metadata;
  audioPlayerReadMetadata(metadata);

  if (strcmp(metadata.artist, lastArtist) != 0) {

    strlcpy(lastArtist, metadata.artist, sizeof(lastArtist));

    lv_label_set_text(artistLabel, metadata.artist);
  }

  if (strcmp(metadata.title, lastTitle) != 0) {

    strlcpy(lastTitle, metadata.title, sizeof(lastTitle));

    lv_label_set_text(titleLabel, metadata.title);
  }
}

// --------------------------------------------------
// LISTA DE ESTAÇÕES
// --------------------------------------------------

static void refreshStationSelection() {
  size_t selectedIndex = audioPlayerGetStationIndex();

  size_t count = STATION_COUNT;

  if (count > MAX_STATION_BUTTONS) {
    count = MAX_STATION_BUTTONS;
  }

  for (size_t i = 0; i < count; i++) {
    if (stationButtons[i] == nullptr) {
      continue;
    }

    if (i == selectedIndex) {
      lv_obj_add_state(stationButtons[i], LV_STATE_CHECKED);
    } else {
      lv_obj_clear_state(stationButtons[i], LV_STATE_CHECKED);
    }
  }
}

static void closeStationList() {
  if (stationPanel == nullptr) {
    return;
  }

  stationListOpen = false;

  lv_obj_add_flag(stationPanel, LV_OBJ_FLAG_HIDDEN);
}

static void openStationList() {
  if (stationPanel == nullptr || stationListOpen || volumePanelOpen) {
    return;
  }

  stationListOpen = true;

  refreshStationSelection();

  lv_obj_clear_flag(stationPanel, LV_OBJ_FLAG_HIDDEN);
  lv_obj_move_foreground(stationPanel);

  size_t selectedIndex = audioPlayerGetStationIndex();

  if (selectedIndex < MAX_STATION_BUTTONS && stationButtons[selectedIndex] != nullptr) {

    lv_obj_scroll_to_view(stationButtons[selectedIndex], LV_ANIM_OFF);
  }
}

static void stationButtonEvent(lv_event_t* event) {

  if (lv_event_get_code(event) != LV_EVENT_CLICKED) {
    return;
  }

  size_t index = static_cast<size_t>(reinterpret_cast<uintptr_t>(lv_event_get_user_data(event)));

  if (index >= STATION_COUNT) {
    return;
  }

  Serial.printf("[Touch] Estacao: %s\n", stations[index].name);

  audioPlayerSelectStation(index);

  refreshStationSelection();
  closeStationList();
}

static void closeButtonEvent(lv_event_t* event) {

  if (lv_event_get_code(event) == LV_EVENT_CLICKED) {

    closeStationList();
  }
}

static void stationGestureEvent(lv_event_t* event) {

  if (lv_event_get_code(event) != LV_EVENT_GESTURE) {
    return;
  }

  lv_indev_t* input = lv_indev_get_act();

  if (input == nullptr) {
    return;
  }

  lv_dir_t direction = lv_indev_get_gesture_dir(input);

  // Abre somente quando o gesto começa no cabeçalho
  if (direction == LV_DIR_BOTTOM && touchStartY < 70 && !stationListOpen && !volumePanelOpen) {
    openStationList();
    lv_indev_wait_release(input);
    return;
  }

  // Gesto para cima fecha a lista
  if (direction == LV_DIR_TOP && stationListOpen) {

    closeStationList();
    lv_indev_wait_release(input);
  }
}

static void createStationList() {
  stationPanel = lv_obj_create(lv_scr_act());

  lv_obj_set_size(stationPanel, 480, 260);
  lv_obj_set_pos(stationPanel, 0, 60);
  lv_obj_set_style_bg_color(stationPanel, lv_color_hex(0x0078D4), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(stationPanel, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_border_width(stationPanel, 0, LV_PART_MAIN);
  lv_obj_set_style_radius(stationPanel, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_all(stationPanel, 0, LV_PART_MAIN);
  lv_obj_clear_flag(stationPanel, LV_OBJ_FLAG_SCROLLABLE);

  // Título
  lv_obj_t* panelTitle = lv_label_create(stationPanel);

  lv_label_set_text(panelTitle, "ESTACOES");
  lv_obj_set_style_text_font(panelTitle, &lv_font_montserrat_20, LV_PART_MAIN);
  lv_obj_set_style_text_color(panelTitle, lv_color_white(), LV_PART_MAIN);
  lv_obj_align(panelTitle, LV_ALIGN_TOP_MID, 0, 10);

  // Botão para fechar
  lv_obj_t* closeButton = lv_btn_create(stationPanel);

  lv_obj_set_size(closeButton, 42, 34);
  lv_obj_align(closeButton, LV_ALIGN_TOP_RIGHT, -6, 4);
  lv_obj_set_style_bg_color(closeButton, lv_color_hex(0x003B73), LV_PART_MAIN);

  lv_obj_add_event_cb(closeButton, closeButtonEvent, LV_EVENT_CLICKED, nullptr);

  lv_obj_t* closeLabel = lv_label_create(closeButton);

  lv_label_set_text(closeLabel, "X");
  lv_obj_set_style_text_color(closeLabel, lv_color_white(), LV_PART_MAIN);
  lv_obj_center(closeLabel);

  // Lista rolável
  stationList = lv_list_create(stationPanel);

  lv_obj_set_size(stationList, 460, 210);
  lv_obj_set_pos(stationList, 10, 45);
  lv_obj_set_style_bg_color(stationList, lv_color_hex(0x0078D4), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(stationList, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_border_width(stationList, 0, LV_PART_MAIN);
  lv_obj_set_style_radius(stationList, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_all(stationList, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_row(stationList, 4, LV_PART_MAIN);

  size_t count = STATION_COUNT;

  if (count > MAX_STATION_BUTTONS) {
    count = MAX_STATION_BUTTONS;
  }

  for (size_t i = 0; i < count; i++) {
    char stationText[96];

    snprintf(stationText, sizeof(stationText), "%u - %s", stations[i].id, stations[i].name);

    lv_obj_t* button = lv_list_add_btn(stationList, nullptr, stationText);

    stationButtons[i] = button;

    lv_obj_set_height(button, 44);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x008B8B), LV_PART_MAIN);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x003B73), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_color(button, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(button, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_set_style_radius(button, 6, LV_PART_MAIN);
    lv_obj_set_style_border_width(button, 0, LV_PART_MAIN);

    lv_obj_add_event_cb(button, stationButtonEvent, LV_EVENT_CLICKED, reinterpret_cast<void*>(static_cast<uintptr_t>(i)));
  }

  refreshStationSelection();

  // Começa escondida
  lv_obj_add_flag(stationPanel, LV_OBJ_FLAG_HIDDEN);

  // Recebe os gestos enviados pelos objetos da tela
  lv_obj_add_event_cb(lv_scr_act(), stationGestureEvent, LV_EVENT_GESTURE, nullptr);
}

// --------------------------------------------------
// PAINEL DE VOLUME
// --------------------------------------------------

static void updateVolumeVisual() {
  uint8_t volume = audioPlayerGetVolume();

  if (volume > 21) {
    volume = 21;
  }

  uint8_t percentage = (volume * 100) / 21;

  char volumeText[8];
  char percentageText[12];

  snprintf(volumeText, sizeof(volumeText), "%02u", volume);
  snprintf(percentageText, sizeof(percentageText), "%u%%", percentage);

  lv_label_set_text(volumeValueLabel, volumeText);
  lv_label_set_text(volumePercentLabel, percentageText);
  lv_label_set_text(volumeTitleLabel, volume == 0 ? "VOLUME - MUDO" : "VOLUME");

  for (uint8_t i = 0; i < 21; i++) {
    lv_color_t color;

    if (i < volume) {
      color = i >= 17 ? lv_color_hex(0xFFFF00) : lv_color_hex(0x00FFFF);
    } else {
      color = lv_color_hex(0x003B73);
    }

    lv_obj_set_style_bg_color(volumeSegments[i], color, LV_PART_MAIN);
  }
}

static void closeVolumePanel() {
  if (volumePanel == nullptr) {
    return;
  }

  volumePanelOpen = false;
  volumeLastInteraction = 0;
  volumeMovementAccumulator = 0;

  lv_obj_add_flag(volumePanel, LV_OBJ_FLAG_HIDDEN);
}

static void openVolumePanel() {
  if (volumePanel == nullptr || volumePanelOpen || stationListOpen) {
    return;
  }

  volumePanelOpen = true;
  volumeLastInteraction = millis();
  volumeMovementAccumulator = 0;

  updateVolumeVisual();

  lv_obj_clear_flag(volumePanel, LV_OBJ_FLAG_HIDDEN);
  lv_obj_move_foreground(volumePanel);
}

static void volumeControlEvent(lv_event_t* event) {

  lv_event_code_t code = lv_event_get_code(event);
  lv_indev_t* input = lv_indev_get_act();

  if (input == nullptr) {
    return;
  }

  lv_point_t point;
  lv_indev_get_point(input, &point);

  if (code == LV_EVENT_PRESSED) {
    volumeLastX = point.x;
    volumeMovementAccumulator = 0;
    volumeLastInteraction = millis();
    return;
  }

  if (code == LV_EVENT_PRESSING) {
    int16_t deltaX =
      point.x - volumeLastX;

    volumeLastX = point.x;

    volumeMovementAccumulator +=
      deltaX;

    int16_t volume = audioPlayerGetVolume();

    bool changed = false;

    while (volumeMovementAccumulator >= PIXELS_PER_VOLUME_STEP) {

      volumeMovementAccumulator -= PIXELS_PER_VOLUME_STEP;

      if (volume < 21) {
        volume++;
        changed = true;
      }
    }

    while (volumeMovementAccumulator <= -PIXELS_PER_VOLUME_STEP) {

      volumeMovementAccumulator += PIXELS_PER_VOLUME_STEP;

      if (volume > 0) {
        volume--;
        changed = true;
      }
    }

    if (changed) {
      audioPlayerSetVolume(static_cast<uint8_t>(volume));

      updateVolumeVisual();
    }

    volumeLastInteraction = millis();
    return;
  }

  if (code == LV_EVENT_RELEASED) {
    volumeMovementAccumulator = 0;
    volumeLastInteraction = millis();
  }
}

static void volumeGestureEvent(lv_event_t* event) {

  if (lv_event_get_code(event) != LV_EVENT_GESTURE) {
    return;
  }

  lv_indev_t* input = lv_indev_get_act();

  if (input == nullptr) {
    return;
  }

  lv_dir_t direction = lv_indev_get_gesture_dir(input);

  if (direction == LV_DIR_RIGHT && touchStartX < 100 && !stationListOpen && !volumePanelOpen) {

    openVolumePanel();
    lv_indev_wait_release(input);
  }
}

static void createVolumePanel() {
  volumePanel = lv_obj_create(lv_scr_act());

  lv_obj_set_size(volumePanel, 480, 260);
  lv_obj_set_pos(volumePanel, 0, 60);

  lv_obj_set_style_bg_color(volumePanel, lv_color_hex(0x0078D4), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(volumePanel, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_border_width(volumePanel, 0, LV_PART_MAIN);
  lv_obj_set_style_radius(volumePanel, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_all(volumePanel, 0, LV_PART_MAIN);
  lv_obj_clear_flag(volumePanel, LV_OBJ_FLAG_SCROLLABLE);

  // Título
  volumeTitleLabel = lv_label_create(volumePanel);

  lv_obj_set_style_text_font(volumeTitleLabel, &lv_font_montserrat_20, LV_PART_MAIN);
  lv_obj_set_style_text_color(volumeTitleLabel, lv_color_white(), LV_PART_MAIN);
  lv_obj_align(volumeTitleLabel, LV_ALIGN_TOP_MID, 0, 12);

  // Valor do volume
  volumeValueLabel = lv_label_create(volumePanel);

  lv_obj_set_style_text_font(volumeValueLabel, &lv_font_montserrat_28, LV_PART_MAIN);
  lv_obj_set_style_text_color(volumeValueLabel, lv_color_white(), LV_PART_MAIN);
  lv_obj_align(volumeValueLabel, LV_ALIGN_CENTER, 0, -42);

  // Porcentagem
  volumePercentLabel = lv_label_create(volumePanel);

  lv_obj_set_style_text_font(volumePercentLabel, &lv_font_montserrat_18, LV_PART_MAIN);
  lv_obj_set_style_text_color(volumePercentLabel, lv_color_hex(0x00FFFF), LV_PART_MAIN);
  lv_obj_align(volumePercentLabel, LV_ALIGN_CENTER, 0, 0);

  // Símbolos
  lv_obj_t* minusLabel = lv_label_create(volumePanel);

  lv_label_set_text(minusLabel, "-");
  lv_obj_set_style_text_font(minusLabel, &lv_font_montserrat_24, LV_PART_MAIN);
  lv_obj_set_style_text_color(minusLabel, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_pos(minusLabel, 55, 178);

  lv_obj_t* plusLabel = lv_label_create(volumePanel);

  lv_label_set_text(plusLabel, "+");
  lv_obj_set_style_text_font(plusLabel, &lv_font_montserrat_24, LV_PART_MAIN);
  lv_obj_set_style_text_color(plusLabel, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_pos(plusLabel, 410, 178);

  // 21 segmentos
  constexpr int16_t startX = 93;
  constexpr int16_t segmentWidth = 10;
  constexpr int16_t segmentGap = 4;

  for (uint8_t i = 0; i < 21; i++) {
    volumeSegments[i] = lv_obj_create(volumePanel);

    lv_obj_set_size(volumeSegments[i], segmentWidth, 18);
    lv_obj_set_pos(volumeSegments[i], startX + i * (segmentWidth + segmentGap), 183);
    lv_obj_set_style_border_width(volumeSegments[i], 0, LV_PART_MAIN);
    lv_obj_set_style_radius(volumeSegments[i], 2, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(volumeSegments[i], LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_clear_flag(volumeSegments[i], LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(volumeSegments[i], LV_OBJ_FLAG_CLICKABLE);
  }

  // Área invisível para ajuste horizontal
  volumeControlArea =
    lv_obj_create(volumePanel);

  lv_obj_set_size(volumeControlArea, 420, 110);
  lv_obj_set_pos(volumeControlArea, 30, 125);
  lv_obj_set_style_bg_opa(volumeControlArea, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_border_width(volumeControlArea, 0, LV_PART_MAIN);
  lv_obj_add_flag(volumeControlArea, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_clear_flag(volumeControlArea, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_event_cb(volumeControlArea, volumeControlEvent, LV_EVENT_ALL, nullptr);

  updateVolumeVisual();

  lv_obj_add_flag(volumePanel, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_event_cb(lv_scr_act(), volumeGestureEvent, LV_EVENT_GESTURE, nullptr);
}

// --------------------------------------------------
// INICIALIZAÇÃO
// --------------------------------------------------

void uiBegin() {
  lcd.init();
  lcd.setRotation(DISPLAY_ROTATION);
  lcd.setBrightness(DISPLAY_BRIGHTNESS);

  lv_init();

  lv_disp_draw_buf_init(&drawBuffer, bufferMemory, nullptr, SCREEN_WIDTH * BUFFER_LINES);

  lv_disp_drv_init(&displayDriver);

  displayDriver.hor_res = SCREEN_WIDTH;
  displayDriver.ver_res = SCREEN_HEIGHT;
  displayDriver.flush_cb = displayFlush;
  displayDriver.draw_buf = &drawBuffer;

  lv_disp_drv_register(&displayDriver);

  lv_indev_drv_init(&touchDriver);

  touchDriver.type = LV_INDEV_TYPE_POINTER;
  touchDriver.read_cb = touchRead;

  lv_indev_drv_register(&touchDriver);

  lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x0078D4), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(lv_scr_act(), LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_clear_flag(lv_scr_act(), LV_OBJ_FLAG_SCROLLABLE);

  createHeader();
  createMetadata();

  statusLabel = lv_label_create(lv_scr_act());

  lv_obj_set_style_text_color(statusLabel, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_text_font(statusLabel, &lv_font_montserrat_20, LV_PART_MAIN);
  lv_obj_align(statusLabel, LV_ALIGN_BOTTOM_MID, 0, -15);

  // Área invisível de Play/Stop
  lv_obj_t* playTouchArea = lv_obj_create(lv_scr_act());

  lv_obj_set_size(playTouchArea, 280, 190);
  lv_obj_set_pos(playTouchArea, 100, 80);
  lv_obj_set_style_bg_opa(playTouchArea, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_border_width(playTouchArea, 0, LV_PART_MAIN);
  lv_obj_clear_flag(playTouchArea, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_event_cb(playTouchArea, playStopEvent, LV_EVENT_CLICKED, nullptr);

  createStationList();
  createVolumePanel();
  updateHeader();
  updateMetadata();

  lastLvglTick = millis();
  lastHeaderUpdate = millis();
}

// --------------------------------------------------
// ATUALIZAÇÃO
// --------------------------------------------------

void uiUpdate() {
  uint32_t now = millis();

  lv_tick_inc(now - lastLvglTick);
  lastLvglTick = now;

  if (audioPlayerHasUpdate()) {
    updateMetadata();
    updateHeader();
  }

  if (now - lastHeaderUpdate >= 1000) {
    lastHeaderUpdate = now;
    updateHeader();
  }

  if (volumePanelOpen && !touchWasPressed && now - volumeLastInteraction >= VOLUME_CLOSE_TIME) {

    closeVolumePanel();
  }

  lv_timer_handler();
}

void uiShowStatus(const char* message) {
  if (statusLabel != nullptr) {
    lv_label_set_text(statusLabel, message);
  }
}

void uiClearStatus() {
  if (statusLabel != nullptr) {
    lv_label_set_text(statusLabel, "");
  }
}

bool uiIsStationListOpen() {
  return stationListOpen;
}