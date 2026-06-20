#pragma once
#include "widgets.h"
#include <pebble.h>
#include <sys/syslimits.h>

#define CURRENT_SETTINGS_VERSION 1
#define SETTINGS_VERSION_PERSIST_KEY 1
#define SETTINGS_PERSIST_KEY 2
#define SETTINGS_EXTRA_PERSIST_KEY 3
#define ALT_CITY_LABEL_LEN 20
// Each info-layout entry is "id:group:size" (e.g. "2:1:1"); five entries plus
// commas need 30 bytes, so allow a little headroom.
#define INFO_LAYOUT_LEN 32
// id:group:size per line. size 0/1/2 = S/M/L. Secondary lines (0,4) small,
// primary lines (1,3) and the time (2) medium.
#define DEFAULT_INFO_LAYOUT "0:0:0,1:0:1,2:1:1,3:2:1,4:2:0"

// Per-line text size (S/M/L). Stored as the third field of each info-layout
// entry; the time line maps these to a proportionally larger font family.
#define INFO_SIZE_S 0
#define INFO_SIZE_M 1
#define INFO_SIZE_L 2
#define INFO_SIZE_DEFAULT INFO_SIZE_M

// default settings
#ifdef PBL_COLOR
#define DEFAULT_TIME_COLOR GColorBlack
#define DEFAULT_SUBTEXT_PRIMARY_COLOR GColorBlack
#define DEFAULT_SUBTEXT_SECONDARY_COLOR GColorDarkGray
#define DEFAULT_BG_COLOR GColorBlack

// night theme defaults
#define DEFAULT_NIGHT_TIME_COLOR GColorFromHEX(0xFFFFFF)
#define DEFAULT_NIGHT_SUBTEXT_PRIMARY_COLOR GColorFromHEX(0xFFFFFF)
#define DEFAULT_NIGHT_SUBTEXT_SECONDARY_COLOR GColorFromHEX(0xAAAAFF)
#define DEFAULT_NIGHT_BG_COLOR GColorBlack
#else
#define DEFAULT_TIME_COLOR GColorBlack
#define DEFAULT_SUBTEXT_PRIMARY_COLOR GColorBlack
#define DEFAULT_SUBTEXT_SECONDARY_COLOR GColorBlack
#define DEFAULT_BG_COLOR GColorBlack

// night theme defaults
#define DEFAULT_NIGHT_TIME_COLOR GColorWhite
#define DEFAULT_NIGHT_SUBTEXT_PRIMARY_COLOR GColorWhite
#define DEFAULT_NIGHT_SUBTEXT_SECONDARY_COLOR GColorWhite
#define DEFAULT_NIGHT_BG_COLOR GColorBlack
#endif

typedef enum { TEMP_UNIT_CELSIUS = 0, TEMP_UNIT_FAHRENHEIT = 1 } TempUnitType;

typedef enum {
  TIME_FORMAT_SYSTEM = 0,    // follow the watch's 12h/24h system setting
  TIME_FORMAT_24H = 1,       // force 24-hour
  TIME_FORMAT_12H = 2,       // force 12-hour, no AM/PM suffix
  TIME_FORMAT_12H_AMPM = 3   // force 12-hour with AM/PM suffix on the main time
} TimeFormatType;

// Color theme struct containing just the resolved (day or night) color fields.
typedef struct {
  GColor timeColor;
  GColor subtextPrimaryColor;
  GColor subtextSecondaryColor;
  GColor bgColor;
} ColorTheme;

// NOTE: StoredSettings must remain a byte-for-byte prefix of Settings (same
// fields, same order) — Settings_saveToStorage() memcpy's sizeof(StoredSettings)
// bytes out of globalSettings, and load reads them straight back in. Append new
// persisted fields to StoredSettingsExtra instead.
typedef struct {
  // text colors
  GColor timeColor;
  GColor subtextPrimaryColor;
  GColor subtextSecondaryColor;

  // decoration colors
  GColor bgColor;

  // night theme colors
  GColor nightTimeColor;
  GColor nightSubtextPrimaryColor;
  GColor nightSubtextSecondaryColor;
  GColor nightBgColor;

  bool useNightTheme;
  bool useLargeFonts;
  bool showLeadingZero;
  TempUnitType tempUnit;
  uint8_t language;

  // Widget slots (stored as format strings)
  char widgetUpperSecondary[WIDGET_TEXT_LEN];
  char widgetUpperPrimary[WIDGET_TEXT_LEN];
  char widgetLowerPrimary[WIDGET_TEXT_LEN];
  char widgetLowerSecondary[WIDGET_TEXT_LEN];

  char altCityLabel[ALT_CITY_LABEL_LEN];
  int16_t altCityUtcOffset;
  char altCity2Label[ALT_CITY_LABEL_LEN];
  int16_t altCity2UtcOffset;
  int16_t localUtcOffset;
  bool usePrimaryFontForAllWidgets;
  char infoLayout[INFO_LAYOUT_LEN];
  uint8_t timeFormat;  // TimeFormatType
} Settings;

typedef struct {
  // text colors
  GColor timeColor;
  GColor subtextPrimaryColor;
  GColor subtextSecondaryColor;

  // decoration colors
  GColor bgColor;

  // night theme colors
  GColor nightTimeColor;
  GColor nightSubtextPrimaryColor;
  GColor nightSubtextSecondaryColor;
  GColor nightBgColor;

  bool useNightTheme;
  bool useLargeFonts;
  bool showLeadingZero;

  TempUnitType tempUnit;
  uint8_t language;

  // Widget slots (stored as format strings)
  char widgetUpperSecondary[WIDGET_TEXT_LEN];
  char widgetUpperPrimary[WIDGET_TEXT_LEN];
  char widgetLowerPrimary[WIDGET_TEXT_LEN];
  char widgetLowerSecondary[WIDGET_TEXT_LEN];
} StoredSettings;

// we're kinda at the limit for stored settings, so new settings get their own
// special struct
typedef struct {
  char altCityLabel[ALT_CITY_LABEL_LEN];
  int16_t altCityUtcOffset;
  char altCity2Label[ALT_CITY_LABEL_LEN];
  int16_t altCity2UtcOffset;
  int16_t localUtcOffset;
  bool usePrimaryFontForAllWidgets;
  char infoLayout[INFO_LAYOUT_LEN];
  uint8_t timeFormat;  // TimeFormatType
} StoredSettingsExtra;

typedef char StoredSettings_must_fit_in_persist_data
    [(sizeof(StoredSettings) <= PERSIST_DATA_MAX_LENGTH) ? 1 : -1];
typedef char StoredSettingsExtra_must_fit_in_persist_data
    [(sizeof(StoredSettingsExtra) <= PERSIST_DATA_MAX_LENGTH) ? 1 : -1];

extern Settings globalSettings;

ColorTheme getCurrentColorTheme();

// Resolve the effective time format from globalSettings.timeFormat.
// settings_is_24h() honours the TIME_FORMAT_SYSTEM choice by falling back to
// the watch's clock_is_24h_style(). settings_show_am_pm() is true only for the
// explicit "12-hour with AM/PM" option (controls the main time line).
bool settings_is_24h(void);
bool settings_show_am_pm(void);

void Settings_init();
void Settings_deinit();
void Settings_loadFromStorage();
void Settings_saveToStorage();
void Settings_updateDynamicSettings();
