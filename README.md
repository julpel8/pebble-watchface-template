# Pebble Watchface Template

A starter template for building a Pebble watchface. It gives you a working,
configurable face — time plus up to five info lines — on a plain background, so
you can focus on drawing your own design. Extracted from **Solar Earth** with the
globe rendering removed.

## What you get

- **Time** rendered on a solid, themeable background.
- **Up to 5 configurable info lines** (incl. the time) — pick what each shows,
  its group (top / center / bottom) and per-line size (S / M / L).
- **Widget tokens**: date, weekday, week, battery, steps / distance / heart rate,
  a second & third time zone, temperature / conditions, sunrise / sunset, and
  more (see the "?" help in the config page, or `CONFIG_WIDGET_HELP` in
  `src/pkjs/index.js`).
- **Optional night theme** that switches text/background colors after sunset.
- **Full i18n** (40+ languages) for dates and labels.
- **Inline HTML config page** (no build step) — background color, info lines,
  language, time format, temperature unit.
- **Weather + sunrise/sunset** fetched on the phone (PebbleKit JS).

## Project layout

| Path | Role |
| --- | --- |
| `src/c/main.c` | App lifecycle, layers, tick, heartbeat |
| `src/c/drawUtils_*.c` | **Background canvas** — start drawing your face here |
| `src/c/info_layout.c` | Time + widget-slot rendering with a readable halo |
| `src/c/widgets.c` | Watch-side token substitution (`{date}`, `{steps}`, …) |
| `src/c/settings.c/.h` | Settings struct, defaults, persistence |
| `src/c/messaging.c` | AppMessage in/out (config ↔ watch) |
| `src/c/solarUtils.c` | Sunrise/sunset + `isNightTime()` (drives the night theme) |
| `src/c/languages.c`, `fonts.c`, `utils.c` | i18n, fonts, time helpers |
| `src/pkjs/index.js` | PebbleKit JS: config page, weather/solar fetch, token pass |
| `src/pkjs/{weather,suncalc,cities,languages}.js` | JS helpers |

## Make it yours

1. **Identity**: in `package.json`, set `name`, `pebble.displayName`, and a fresh
   `pebble.uuid` (run `uuidgen`). The current uuid was generated for this copy —
   change it before publishing.
2. **Capabilities**: `package.json > pebble.capabilities` is
   `[location, configurable, health]`. Drop `location` if you remove weather/
   sunrise-sunset; drop `health` if you remove the steps/distance/heart-rate
   widgets.
3. **Draw your face**: edit `draw_center_layer()` in `src/c/drawUtils_rect.c`
   (and `_round.c` for round watches).
4. **Add a setting**: add a `messageKeys` entry in `package.json`, a field in
   `Settings`/`StoredSettings(Extra)` (`settings.h`), read it in
   `inbox_received_callback` (`messaging.c`), and expose it in the inline config
   HTML in `src/pkjs/index.js`.

## Build & run

```sh
pebble build
pebble install --emulator emery
```

Target platform is `emery` (`package.json > pebble.targetPlatforms`); add the
others you need. Check translations with `npm run check:i18n`.

---

Clock layout and widgets inspired by Freakified's **Halcyon**. MIT licensed.
