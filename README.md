# Pebble Watchface Template

A minimal starter for building a Pebble watchface: time plus up to five
configurable info lines (date, weather, sunrise/sunset, steps, time zones…) on a
plain, themeable background, with an optional night theme, full i18n, and an
inline HTML config page. Extracted from **Solar Earth**, globe rendering removed —
draw your own face in `src/c/drawUtils_*.c`.

```sh
pebble build
pebble install --emulator emery
```

Before publishing, set `name`, `displayName`, and a fresh `uuid` in
`package.json`. MIT licensed.
