#include "settings.h"
#include "solarUtils.h"
#include "utils.h"
#include "widgets.h"
#include <pebble.h>

Settings globalSettings;

static void populateStoredSettingsExtra(StoredSettingsExtra *storedSettingsExtra) {
  strncpy(storedSettingsExtra->altCityLabel, globalSettings.altCityLabel,
          ALT_CITY_LABEL_LEN);
  storedSettingsExtra->altCityLabel[ALT_CITY_LABEL_LEN - 1] = '\0';
  storedSettingsExtra->altCityUtcOffset = globalSettings.altCityUtcOffset;
  strncpy(storedSettingsExtra->altCity2Label, globalSettings.altCity2Label,
          ALT_CITY_LABEL_LEN);
  storedSettingsExtra->altCity2Label[ALT_CITY_LABEL_LEN - 1] = '\0';
  storedSettingsExtra->altCity2UtcOffset = globalSettings.altCity2UtcOffset;
  storedSettingsExtra->localUtcOffset = globalSettings.localUtcOffset;
  storedSettingsExtra->usePrimaryFontForAllWidgets =
      globalSettings.usePrimaryFontForAllWidgets;
  strncpy(storedSettingsExtra->infoLayout, globalSettings.infoLayout,
          INFO_LAYOUT_LEN);
  storedSettingsExtra->infoLayout[INFO_LAYOUT_LEN - 1] = '\0';
  storedSettingsExtra->timeFormat = globalSettings.timeFormat;
}

void Settings_init() { Settings_loadFromStorage(); }

void Settings_deinit() { Settings_saveToStorage(); }

void Settings_loadFromStorage() {
  // set all the defaults!
  // text colors
  globalSettings.timeColor = DEFAULT_TIME_COLOR;
  globalSettings.subtextPrimaryColor = DEFAULT_SUBTEXT_PRIMARY_COLOR;
  globalSettings.subtextSecondaryColor = DEFAULT_SUBTEXT_SECONDARY_COLOR;

  // decoration colors
  globalSettings.bgColor = DEFAULT_BG_COLOR;

  // night theme colors
  globalSettings.nightTimeColor = DEFAULT_NIGHT_TIME_COLOR;
  globalSettings.nightSubtextPrimaryColor = DEFAULT_NIGHT_SUBTEXT_PRIMARY_COLOR;
  globalSettings.nightSubtextSecondaryColor =
      DEFAULT_NIGHT_SUBTEXT_SECONDARY_COLOR;
  globalSettings.nightBgColor = DEFAULT_NIGHT_BG_COLOR;

  // various appearance settings
  globalSettings.useNightTheme = true;
  globalSettings.useLargeFonts = false;
  globalSettings.showLeadingZero = false;
  globalSettings.tempUnit = TEMP_UNIT_CELSIUS;
  globalSettings.language = 0;
  globalSettings.timeFormat = TIME_FORMAT_SYSTEM;

  // widget slot defaults
  // Weather-dependent slots use placeholders until JS sends real data.
  // This prevents raw tokens like "{temp}°" from flashing on first run.
  strncpy(globalSettings.widgetUpperSecondary, "--° (--° / --°)",
          WIDGET_TEXT_LEN);
  strncpy(globalSettings.widgetUpperPrimary, "--", WIDGET_TEXT_LEN);
  // Lower-primary defaults to the {local_date} super-token; widgets.c expands
  // it at render time using the active language's idiomatic format.
  strncpy(globalSettings.widgetLowerPrimary, "{local_date}", WIDGET_TEXT_LEN);
#if defined(PBL_HEALTH)
  strncpy(globalSettings.widgetLowerSecondary, "{steps} {t:STEPS}",
          WIDGET_TEXT_LEN);
#else
  strncpy(globalSettings.widgetLowerSecondary, "{t:BATTERY} {batt}%",
          WIDGET_TEXT_LEN);
#endif
  strncpy(globalSettings.altCityLabel, "TYO", ALT_CITY_LABEL_LEN);
  globalSettings.altCityLabel[ALT_CITY_LABEL_LEN - 1] = '\0';
  globalSettings.altCityUtcOffset = 540;
  strncpy(globalSettings.altCity2Label, "UTC", ALT_CITY_LABEL_LEN);
  globalSettings.altCity2Label[ALT_CITY_LABEL_LEN - 1] = '\0';
  globalSettings.altCity2UtcOffset = 0;
  globalSettings.localUtcOffset = 0;
  globalSettings.usePrimaryFontForAllWidgets = false;
  strncpy(globalSettings.infoLayout, DEFAULT_INFO_LAYOUT, INFO_LAYOUT_LEN);
  globalSettings.infoLayout[INFO_LAYOUT_LEN - 1] = '\0';

  if (persist_exists(SETTINGS_PERSIST_KEY)) {
    const int stored_size = persist_get_size(SETTINGS_PERSIST_KEY);
    if (stored_size > 0) {
      // Settings storage is append-only: defaults above cover fields that did
      // not exist in older persisted blobs.
      const int read_size = stored_size < (int)sizeof(StoredSettings)
                                ? stored_size
                                : (int)sizeof(StoredSettings);
      persist_read_data(SETTINGS_PERSIST_KEY, &globalSettings, read_size);
    }
  }

  if (persist_exists(SETTINGS_EXTRA_PERSIST_KEY)) {
    const int stored_size = persist_get_size(SETTINGS_EXTRA_PERSIST_KEY);
    if (stored_size > 0) {
      StoredSettingsExtra storedSettingsExtra;
      memset(&storedSettingsExtra, 0, sizeof(StoredSettingsExtra));
      populateStoredSettingsExtra(&storedSettingsExtra);
      const int read_size = stored_size < (int)sizeof(StoredSettingsExtra)
                                ? stored_size
                                : (int)sizeof(StoredSettingsExtra);
      persist_read_data(SETTINGS_EXTRA_PERSIST_KEY, &storedSettingsExtra,
                        read_size);

      strncpy(globalSettings.altCityLabel, storedSettingsExtra.altCityLabel,
              ALT_CITY_LABEL_LEN);
      globalSettings.altCityLabel[ALT_CITY_LABEL_LEN - 1] = '\0';
      globalSettings.altCityUtcOffset = storedSettingsExtra.altCityUtcOffset;
      strncpy(globalSettings.altCity2Label, storedSettingsExtra.altCity2Label,
              ALT_CITY_LABEL_LEN);
      globalSettings.altCity2Label[ALT_CITY_LABEL_LEN - 1] = '\0';
      globalSettings.altCity2UtcOffset = storedSettingsExtra.altCity2UtcOffset;
      globalSettings.localUtcOffset = storedSettingsExtra.localUtcOffset;
      globalSettings.usePrimaryFontForAllWidgets =
          storedSettingsExtra.usePrimaryFontForAllWidgets;
      strncpy(globalSettings.infoLayout, storedSettingsExtra.infoLayout,
              INFO_LAYOUT_LEN);
      globalSettings.infoLayout[INFO_LAYOUT_LEN - 1] = '\0';
      globalSettings.timeFormat = storedSettingsExtra.timeFormat;
    }
  }

  // Guard against an empty/corrupt persisted info layout.
  if (globalSettings.infoLayout[0] == '\0') {
    strncpy(globalSettings.infoLayout, DEFAULT_INFO_LAYOUT, INFO_LAYOUT_LEN);
    globalSettings.infoLayout[INFO_LAYOUT_LEN - 1] = '\0';
  }

  Settings_updateDynamicSettings();
}

void Settings_saveToStorage() {
  Settings_updateDynamicSettings();
  StoredSettings storedSettings;
  memcpy(&storedSettings, &globalSettings, sizeof(StoredSettings));
  persist_write_data(SETTINGS_PERSIST_KEY, &storedSettings,
                     sizeof(StoredSettings));

  StoredSettingsExtra storedSettingsExtra;
  memset(&storedSettingsExtra, 0, sizeof(StoredSettingsExtra));
  populateStoredSettingsExtra(&storedSettingsExtra);
  persist_write_data(SETTINGS_EXTRA_PERSIST_KEY, &storedSettingsExtra,
                     sizeof(StoredSettingsExtra));

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

  // Get current time in minutes since midnight
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
