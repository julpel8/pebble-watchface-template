#include "settings.h"
#include "solarUtils.h"
#include "utils.h"
#include "widgets.h"
#include <pebble.h>

Settings globalSettings;

// ---------------------------------------------------------------------------
// Per-field persist keys. Each setting owns its own key, so adding or removing
// a field never disturbs the others. Numbered from 100 to stay clear of the
// solar data key (51).
// ---------------------------------------------------------------------------
enum {
  PK_TIME_COLOR = 100,
  PK_SUBTEXT_PRIMARY_COLOR,
  PK_SUBTEXT_SECONDARY_COLOR,
  PK_BG_COLOR,
  PK_NIGHT_TIME_COLOR,
  PK_NIGHT_SUBTEXT_PRIMARY_COLOR,
  PK_NIGHT_SUBTEXT_SECONDARY_COLOR,
  PK_NIGHT_BG_COLOR,
  PK_USE_NIGHT_THEME,
  PK_USE_LARGE_FONTS,
  PK_SHOW_LEADING_ZERO,
  PK_USE_PRIMARY_FONT,
  PK_TEMP_UNIT,
  PK_LANGUAGE,
  PK_TIME_FORMAT,
  PK_ALT_CITY_LABEL,
  PK_ALT_CITY_UTC_OFFSET,
  PK_ALT_CITY2_LABEL,
  PK_ALT_CITY2_UTC_OFFSET,
  PK_INFO_LAYOUT,
  PK_WIDGET_UPPER_SECONDARY,
  PK_WIDGET_UPPER_PRIMARY,
  PK_WIDGET_LOWER_PRIMARY,
  PK_WIDGET_LOWER_SECONDARY,
};

// ---------------------------------------------------------------------------
// Small persist helpers
// ---------------------------------------------------------------------------
static void persist_put_color(uint32_t key, GColor c) {
  persist_write_data(key, &c, sizeof(GColor));
}

static GColor persist_get_color(uint32_t key, GColor def) {
  GColor c = def;
  if (persist_exists(key)) {
    persist_read_data(key, &c, sizeof(GColor));
  }
  return c;
}

static int persist_get_int(uint32_t key, int def) {
  return persist_exists(key) ? persist_read_int(key) : def;
}

static bool persist_get_bool(uint32_t key, bool def) {
  return persist_exists(key) ? persist_read_bool(key) : def;
}

// Overwrite buf only if the key exists; otherwise leave the caller's default.
static void persist_load_str(uint32_t key, char *buf, size_t buf_len) {
  if (persist_exists(key)) {
    persist_read_string(key, buf, buf_len);
    buf[buf_len - 1] = '\0';
  }
}

// ---------------------------------------------------------------------------
// Defaults / load / save
// ---------------------------------------------------------------------------
static void set_defaults(void) {
  memset(&globalSettings, 0, sizeof(globalSettings));

  globalSettings.timeColor = DEFAULT_TIME_COLOR;
  globalSettings.subtextPrimaryColor = DEFAULT_SUBTEXT_PRIMARY_COLOR;
  globalSettings.subtextSecondaryColor = DEFAULT_SUBTEXT_SECONDARY_COLOR;
  globalSettings.bgColor = DEFAULT_BG_COLOR;

  globalSettings.nightTimeColor = DEFAULT_NIGHT_TIME_COLOR;
  globalSettings.nightSubtextPrimaryColor = DEFAULT_NIGHT_SUBTEXT_PRIMARY_COLOR;
  globalSettings.nightSubtextSecondaryColor =
      DEFAULT_NIGHT_SUBTEXT_SECONDARY_COLOR;
  globalSettings.nightBgColor = DEFAULT_NIGHT_BG_COLOR;

  globalSettings.useNightTheme = true;
  globalSettings.useLargeFonts = false;
  globalSettings.showLeadingZero = false;
  globalSettings.usePrimaryFontForAllWidgets = false;
  globalSettings.tempUnit = TEMP_UNIT_CELSIUS;
  globalSettings.language = 0;
  globalSettings.timeFormat = TIME_FORMAT_SYSTEM;

  // Weather-dependent slots use placeholders until JS sends real data, so raw
  // tokens like "{temp}°" don't flash on first run.
  strncpy(globalSettings.widgetUpperSecondary, "--° (--° / --°)",
          WIDGET_TEXT_LEN);
  strncpy(globalSettings.widgetUpperPrimary, "--", WIDGET_TEXT_LEN);
  strncpy(globalSettings.widgetLowerPrimary, "{local_date}", WIDGET_TEXT_LEN);
#if defined(PBL_HEALTH)
  strncpy(globalSettings.widgetLowerSecondary, "{steps} {t:STEPS}",
          WIDGET_TEXT_LEN);
#else
  strncpy(globalSettings.widgetLowerSecondary, "{t:BATTERY} {batt}%",
          WIDGET_TEXT_LEN);
#endif

  strncpy(globalSettings.altCityLabel, "TYO", ALT_CITY_LABEL_LEN);
  globalSettings.altCityUtcOffset = 540;
  strncpy(globalSettings.altCity2Label, "UTC", ALT_CITY_LABEL_LEN);
  globalSettings.altCity2UtcOffset = 0;
  globalSettings.localUtcOffset = 0;

  strncpy(globalSettings.infoLayout, DEFAULT_INFO_LAYOUT, INFO_LAYOUT_LEN);
}

static void load_from_keys(void) {
  globalSettings.timeColor =
      persist_get_color(PK_TIME_COLOR, globalSettings.timeColor);
  globalSettings.subtextPrimaryColor =
      persist_get_color(PK_SUBTEXT_PRIMARY_COLOR, globalSettings.subtextPrimaryColor);
  globalSettings.subtextSecondaryColor = persist_get_color(
      PK_SUBTEXT_SECONDARY_COLOR, globalSettings.subtextSecondaryColor);
  globalSettings.bgColor = persist_get_color(PK_BG_COLOR, globalSettings.bgColor);

  globalSettings.nightTimeColor =
      persist_get_color(PK_NIGHT_TIME_COLOR, globalSettings.nightTimeColor);
  globalSettings.nightSubtextPrimaryColor = persist_get_color(
      PK_NIGHT_SUBTEXT_PRIMARY_COLOR, globalSettings.nightSubtextPrimaryColor);
  globalSettings.nightSubtextSecondaryColor = persist_get_color(
      PK_NIGHT_SUBTEXT_SECONDARY_COLOR, globalSettings.nightSubtextSecondaryColor);
  globalSettings.nightBgColor =
      persist_get_color(PK_NIGHT_BG_COLOR, globalSettings.nightBgColor);

  globalSettings.useNightTheme =
      persist_get_bool(PK_USE_NIGHT_THEME, globalSettings.useNightTheme);
  globalSettings.useLargeFonts =
      persist_get_bool(PK_USE_LARGE_FONTS, globalSettings.useLargeFonts);
  globalSettings.showLeadingZero =
      persist_get_bool(PK_SHOW_LEADING_ZERO, globalSettings.showLeadingZero);
  globalSettings.usePrimaryFontForAllWidgets = persist_get_bool(
      PK_USE_PRIMARY_FONT, globalSettings.usePrimaryFontForAllWidgets);

  globalSettings.tempUnit =
      (TempUnitType)persist_get_int(PK_TEMP_UNIT, globalSettings.tempUnit);
  globalSettings.language =
      (uint8_t)persist_get_int(PK_LANGUAGE, globalSettings.language);
  globalSettings.timeFormat =
      (uint8_t)persist_get_int(PK_TIME_FORMAT, globalSettings.timeFormat);

  globalSettings.altCityUtcOffset = (int16_t)persist_get_int(
      PK_ALT_CITY_UTC_OFFSET, globalSettings.altCityUtcOffset);
  globalSettings.altCity2UtcOffset = (int16_t)persist_get_int(
      PK_ALT_CITY2_UTC_OFFSET, globalSettings.altCity2UtcOffset);

  persist_load_str(PK_ALT_CITY_LABEL, globalSettings.altCityLabel,
                   ALT_CITY_LABEL_LEN);
  persist_load_str(PK_ALT_CITY2_LABEL, globalSettings.altCity2Label,
                   ALT_CITY_LABEL_LEN);
  persist_load_str(PK_INFO_LAYOUT, globalSettings.infoLayout, INFO_LAYOUT_LEN);
  persist_load_str(PK_WIDGET_UPPER_SECONDARY,
                   globalSettings.widgetUpperSecondary, WIDGET_TEXT_LEN);
  persist_load_str(PK_WIDGET_UPPER_PRIMARY, globalSettings.widgetUpperPrimary,
                   WIDGET_TEXT_LEN);
  persist_load_str(PK_WIDGET_LOWER_PRIMARY, globalSettings.widgetLowerPrimary,
                   WIDGET_TEXT_LEN);
  persist_load_str(PK_WIDGET_LOWER_SECONDARY,
                   globalSettings.widgetLowerSecondary, WIDGET_TEXT_LEN);
}

void Settings_init() { Settings_loadFromStorage(); }

void Settings_deinit() { Settings_saveToStorage(); }

void Settings_loadFromStorage() {
  set_defaults();
  load_from_keys();  // each key is optional; missing keys keep their default

  // Guard against an empty/corrupt info layout.
  if (globalSettings.infoLayout[0] == '\0') {
    strncpy(globalSettings.infoLayout, DEFAULT_INFO_LAYOUT, INFO_LAYOUT_LEN);
    globalSettings.infoLayout[INFO_LAYOUT_LEN - 1] = '\0';
  }

  Settings_updateDynamicSettings();
}

void Settings_saveToStorage() {
  Settings_updateDynamicSettings();

  persist_put_color(PK_TIME_COLOR, globalSettings.timeColor);
  persist_put_color(PK_SUBTEXT_PRIMARY_COLOR, globalSettings.subtextPrimaryColor);
  persist_put_color(PK_SUBTEXT_SECONDARY_COLOR,
                    globalSettings.subtextSecondaryColor);
  persist_put_color(PK_BG_COLOR, globalSettings.bgColor);
  persist_put_color(PK_NIGHT_TIME_COLOR, globalSettings.nightTimeColor);
  persist_put_color(PK_NIGHT_SUBTEXT_PRIMARY_COLOR,
                    globalSettings.nightSubtextPrimaryColor);
  persist_put_color(PK_NIGHT_SUBTEXT_SECONDARY_COLOR,
                    globalSettings.nightSubtextSecondaryColor);
  persist_put_color(PK_NIGHT_BG_COLOR, globalSettings.nightBgColor);

  persist_write_bool(PK_USE_NIGHT_THEME, globalSettings.useNightTheme);
  persist_write_bool(PK_USE_LARGE_FONTS, globalSettings.useLargeFonts);
  persist_write_bool(PK_SHOW_LEADING_ZERO, globalSettings.showLeadingZero);
  persist_write_bool(PK_USE_PRIMARY_FONT,
                     globalSettings.usePrimaryFontForAllWidgets);

  persist_write_int(PK_TEMP_UNIT, globalSettings.tempUnit);
  persist_write_int(PK_LANGUAGE, globalSettings.language);
  persist_write_int(PK_TIME_FORMAT, globalSettings.timeFormat);
  persist_write_int(PK_ALT_CITY_UTC_OFFSET, globalSettings.altCityUtcOffset);
  persist_write_int(PK_ALT_CITY2_UTC_OFFSET, globalSettings.altCity2UtcOffset);

  persist_write_string(PK_ALT_CITY_LABEL, globalSettings.altCityLabel);
  persist_write_string(PK_ALT_CITY2_LABEL, globalSettings.altCity2Label);
  persist_write_string(PK_INFO_LAYOUT, globalSettings.infoLayout);
  persist_write_string(PK_WIDGET_UPPER_SECONDARY,
                       globalSettings.widgetUpperSecondary);
  persist_write_string(PK_WIDGET_UPPER_PRIMARY,
                       globalSettings.widgetUpperPrimary);
  persist_write_string(PK_WIDGET_LOWER_PRIMARY,
                       globalSettings.widgetLowerPrimary);
  persist_write_string(PK_WIDGET_LOWER_SECONDARY,
                       globalSettings.widgetLowerSecondary);

  persist_write_int(SETTINGS_VERSION_PERSIST_KEY, CURRENT_SETTINGS_VERSION);
}

static int16_t get_local_utc_offset(void) {
  time_t now = time(NULL);
  struct tm local_time = *localtime(&now);
  struct tm utc_time = *gmtime(&now);

  int day_delta = local_time.tm_yday - utc_time.tm_yday;
  if (local_time.tm_year > utc_time.tm_year) {
    day_delta = 1;
  } else if (local_time.tm_year < utc_time.tm_year) {
    day_delta = -1;
  } else if (day_delta > 1) {
    day_delta = -1;
  } else if (day_delta < -1) {
    day_delta = 1;
  }

  return (int16_t)((day_delta * 24 + local_time.tm_hour - utc_time.tm_hour) *
                       60 +
                   local_time.tm_min - utc_time.tm_min);
}

void Settings_updateDynamicSettings() {
  globalSettings.localUtcOffset = get_local_utc_offset();
}

bool settings_is_24h(void) {
  switch ((TimeFormatType)globalSettings.timeFormat) {
    case TIME_FORMAT_24H:
      return true;
    case TIME_FORMAT_12H:
    case TIME_FORMAT_12H_AMPM:
      return false;
    case TIME_FORMAT_SYSTEM:
    default:
      return clock_is_24h_style();
  }
}

bool settings_show_am_pm(void) {
  return (TimeFormatType)globalSettings.timeFormat == TIME_FORMAT_12H_AMPM;
}

ColorTheme getCurrentColorTheme() {
  ColorTheme theme;

  struct tm *timeInfo = getCurrentTime();
  int currentMinutes = timeInfo->tm_hour * 60 + timeInfo->tm_min;

  bool useNight = globalSettings.useNightTheme && isNightTime(currentMinutes);

  if (useNight) {
    theme.timeColor = globalSettings.nightTimeColor;
    theme.subtextPrimaryColor = globalSettings.nightSubtextPrimaryColor;
    theme.subtextSecondaryColor = globalSettings.nightSubtextSecondaryColor;
    theme.bgColor = globalSettings.nightBgColor;
  } else {
    theme.timeColor = globalSettings.timeColor;
    theme.subtextPrimaryColor = globalSettings.subtextPrimaryColor;
    theme.subtextSecondaryColor = globalSettings.subtextSecondaryColor;
    theme.bgColor = globalSettings.bgColor;
  }

  return theme;
}
