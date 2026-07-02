## Plan

1. Add compile-time SDMMC options and broaden `USE_SD`.
2. Refactor `SDManager` to support both `SD` and `SD_MMC`.
3. Update config integration to use the manager's active filesystem.
4. Enable built-in SDMMC pins in the ESP32-S3 board config.
5. Compile, fix errors, and flash the board.
