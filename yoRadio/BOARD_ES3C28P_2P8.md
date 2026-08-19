# LCDWIKI ES3C28P / ESP32-S3 2.8 Touch

This project tree contains a custom profile for the `2.8" LCD Display ESP32-S3 240x320 Capacitive Touch` board, also seen as `LCDWIKI ES3C28P`.

## Hardware summary

- MCU: `ESP32-S3`
- Arduino core: `esp32` 3.x (required by ESP32-audioI2S 3.4.7)
- PSRAM: `OPI PSRAM` (required by ESP32-audioI2S 3.4.7)
- LCD controller: `ILI9341V`
- Visible layout in yoRadio: portrait `240x320`
- Touch controller: `FT6336G`
- Storage: onboard `SD_MMC` slot
- I2S pinout for the onboard ES8311 or an external I2S DAC:
  - `BCLK -> GPIO43`
  - `LRC/WS -> GPIO44`
  - `DOUT -> GPIO21`
  - `MCLK -> not used`

## Pins used by this profile

### LCD SPI

- `SCK  -> GPIO12`
- `MISO -> GPIO13`
- `MOSI -> GPIO11`
- `CS   -> GPIO10`
- `DC   -> GPIO46`
- `RST  -> -1` (shared / not used separately)
- `BL   -> GPIO45`

### Touch FT6336G

- `SDA  -> GPIO16`
- `SCL  -> GPIO15`
- `RST  -> GPIO18`
- `INT  -> GPIO17`
- `ADDR -> 0x38`

### SD_MMC

- `CLK -> GPIO38`
- `CMD -> GPIO40`
- `D0  -> GPIO39`
- `D1  -> GPIO41`
- `D2  -> GPIO48`
- `D3  -> GPIO47`

### Optional encoder breakout kept in comments

The currently preferred `myoptions.h` leaves the encoder disabled, but keeps the tested mapping in comments:

- `ENC_BTNL -> GPIO2`
- `ENC_BTNB -> GPIO3`
- `ENC_BTNR -> GPIO14`

## Files that define this board

- Board pin configuration: [myoptions.h](myoptions.h)
- Theme palette: [mytheme.h](mytheme.h)
- Custom display layout: [displayILI9341conf_custom.h](src/displays/conf/displayILI9341conf_custom.h)
- Arduino IDE, audio and flashing guide: [ARDUINO_IDE_FLASHING_ES3C28P.md](ARDUINO_IDE_FLASHING_ES3C28P.md)

## `myoptions.h` notes

This board profile currently assumes:

- `L10N_LANGUAGE RU`
- `DSP_MODEL DSP_ILI9341`
- custom LCD SPI pins via `DSP_SPIPINS 12, 13, 11`
- default display inversion enabled
- default flip enabled
- FT6336 touch enabled
- SD card through built-in `SD_MMC`
- onboard ES8311 (when `USE_ES8311` is enabled) or an external I2S DAC on `43/44/21`
- encoder disabled by default, but ready to restore from commented lines

If you want to switch back to another audio path, update `I2S_*` defines in `myoptions.h`.

## `mytheme.h` notes

This branch keeps the board theme separate from hardware settings:

- `myoptions.h` contains board and pin configuration
- `mytheme.h` contains only palette overrides

The current palette is a cyan LCD-style theme built on top of the default yoRadio color set. If the UI looks inconsistent, check for hardcoded UI elements outside the theme file, such as boot graphics or bitmap icons.

## Boot screen assets

- Boot logo for this panel uses a `99x64` RGB565 bitmap
- The current boot logo header is generated from `Yoradio_Logo.png` in the workspace root
- Weather icons are stored separately and are not controlled by `mytheme.h`

## Current branch intent

This profile is meant to keep the original yoRadio structure while carrying a practical, documented setup for the ESP32-S3 2.8" touch board with:

- Russian UI
- custom ILI9341 layout
- FT6336 touch
- SD_MMC support
- theme overrides in `mytheme.h`
- board options in `myoptions.h`
