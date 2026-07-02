## Goal

Add native built-in microSD support for the ESP32-S3 2.8" LCD board in yoRadio using `SD_MMC`, while preserving the existing SPI SD path for other boards.

## Context

- This board's integrated card reader is wired to ESP32-S3 `SD_MMC`, not SPI.
- Existing yoRadio SD support assumes `SD.h` with `SDC_CS`.
- Playlist code already works through generic `FS` APIs once the card is mounted.

## Design

1. Introduce board options for built-in `SD_MMC`:
   - `SDMMC_INTERNAL`
   - `SDMMC_1BIT`
   - `SDC_CLK`, `SDC_CMD`, `SDC_D0`, `SDC_D1`, `SDC_D2`, `SDC_D3`
2. Define `USE_SD` when either SPI SD or `SD_MMC` is enabled.
3. Refactor `SDManager` from `SDFS` inheritance to a small wrapper that exposes:
   - `start()`, `stop()`, `cardPresent()`
   - `open()`, `exists()`, `remove()`
   - `filesystem()`
4. Keep SPI SD behavior intact and add `SD_MMC` startup with configurable pins.
5. Update board defaults for this ESP32-S3 LCD board to use the built-in microSD slot.

## Verification

- Compile `yoRadio` for `esp32:esp32:esp32s3:PartitionScheme=huge_app`.
- If compile succeeds, flash firmware to the same board on `COM5`.
