# yoRadio для ESP32-S3 2.8" Touch: прошивка через Arduino IDE

Эта инструкция относится к нашей плате:

- `2.8" LCD Display ESP32-S3 240x320 Capacitive Touch`
- LCD-контроллер: `ILI9341V`
- touch: `FT6336G`

Проект действительно можно прошивать через `Arduino IDE`.

## 1. Что именно открывать

Открывать нужно файл:

- `F:\My\YoRadio\2.8 touch\yoradio\yoRadio\yoRadio.ino`

Главные пользовательские файлы в этом проекте:

- [myoptions.h](F:/My/YoRadio/2.8%20touch/yoradio/yoRadio/myoptions.h) - пины, дисплей, тач, аудио, SD
- [mytheme.h](F:/My/YoRadio/2.8%20touch/yoradio/yoRadio/mytheme.h) - тема, цвета интерфейса
- [BOARD_ES3C28P_2P8.md](F:/My/YoRadio/2.8%20touch/yoradio/yoRadio/BOARD_ES3C28P_2P8.md) - краткое описание профиля платы

## 2. Что поставить на ПК заранее

Нужно:

1. `Arduino IDE`
2. пакет плат `esp32 by Espressif Systems`
3. несколько библиотек из Library Manager

Рекомендованная версия пакета плат, на которой этот проект у нас собирался:

- `esp32 by Espressif Systems`
- версия `2.0.17`

## 3. Какие библиотеки нужны

Через `Library Manager` проверь, что установлены:

- `Adafruit GFX Library`
- `Adafruit BusIO`
- `Adafruit ILI9341`

Остальная основная логика yoRadio уже лежит внутри проекта.

## 4. Как добавить пакет плат ESP32

Если пакет `esp32` ещё не установлен:

1. Открой `Arduino IDE`
2. Зайди в `File -> Preferences`
3. В поле `Additional boards manager URLs` добавь:

```text
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```

4. Нажми `OK`
5. Открой `Tools -> Board -> Boards Manager`
6. Найди `esp32 by Espressif Systems`
7. Установи версию `2.0.17`

## 5. Как открыть проект

1. Распакуй или положи проект на диск
2. Открой файл `yoRadio.ino`
3. Arduino IDE сама откроет весь каталог скетча

Рабочая папка проекта:

- `F:\My\YoRadio\2.8 touch\yoradio\yoRadio`

## 6. Точные настройки в Arduino IDE

В `Tools` выставить так:

- `Board` -> `ESP32S3 Dev Module`
- `Port` -> COM-порт твоей платы
- `USB Mode` -> `Hardware CDC and JTAG`
- `USB CDC On Boot` -> `Disabled`
- `Upload Mode` -> `UART0 / Hardware CDC`
- `CPU Frequency` -> `240MHz (WiFi)`
- `Flash Mode` -> `QIO 80MHz`
- `Flash Size` -> `4MB (32Mb)`
- `Partition Scheme` -> `Huge APP (3MB No OTA/1MB SPIFFS)`
- `PSRAM` -> `OPI PSRAM`
- `Arduino Runs On` -> `Core 1`
- `Events Run On` -> `Core 1`
- `Upload Speed` -> `921600`
- `Core Debug Level` -> `None`
- `Erase All Flash Before Sketch Upload` -> `Disabled`

Это именно те параметры, с которыми проект у нас собирался и прошивался.

## 7. Что уже зашито в `myoptions.h`

Для этой платы в проекте уже настроено:

- дисплей `DSP_ILI9341`
- инверсия экрана включена по умолчанию
- поворот экрана на `180` включён по умолчанию
- touch `FT6336G` включён
- встроенный кардридер включён через `SD_MMC`
- русский язык интерфейса включён

Текущие пины дисплея:

- `SCK  = GPIO12`
- `MISO = GPIO13`
- `MOSI = GPIO11`
- `CS   = GPIO10`
- `DC   = GPIO46`
- `BL   = GPIO45`

Текущие пины touch:

- `SDA  = GPIO16`
- `SCL  = GPIO15`
- `RST  = GPIO18`
- `INT  = GPIO17`
- `ADDR = 0x38`

Текущая аудиоконфигурация в этой ветке:

- внешний I2S DAC
- `BCLK = GPIO43`
- `LRC  = GPIO44`
- `DOUT = GPIO21`

Если ты меняешь железо, обычно править нужно именно [myoptions.h](F:/My/YoRadio/2.8%20touch/yoradio/yoRadio/myoptions.h).

## 8. Где менять тему

Если хочешь менять цвета и стиль интерфейса, править нужно:

- [mytheme.h](F:/My/YoRadio/2.8%20touch/yoradio/yoRadio/mytheme.h)

Если меняешь только тему:

- скетч перепрошить нужно
- `SPIFFS` обычно трогать не нужно

## 9. Первая прошивка с нуля через Arduino IDE

Рекомендованный сценарий такой:

1. Подключи плату по USB
2. Открой `yoRadio.ino`
3. Выставь все параметры из раздела 6
4. Выбери нужный `COM`-порт
5. Нажми `Verify`
6. Если сборка прошла, нажми `Upload`
7. Дождись окончания прошивки
8. После этого отдельно запиши `SPIFFS`, если флеш был пустой или очищался полностью

Скорость `Serial Monitor`:

- `115200`

## 10. Когда обязательно нужен SPIFFS

`SPIFFS` нужен, потому что yoRadio использует файлы из папки `data`.

Его обязательно прошивать, если:

- это первая прошивка платы
- ты делал полную очистку флеша
- пропала веб-морда
- пропали ресурсы из `data`
- менялись web-файлы, ресурсы, иконки, данные из папки `data`

Если ты менял только код `.ino/.cpp/.h`, часто достаточно прошить только скетч.

## 11. Готовые бинарники

В корне рабочей папки уже лежат готовые `.bin`:

- [yoradio-2p8-touch-public-bootloader.bin](F:/My/YoRadio/2.8%20touch/yoradio-2p8-touch-public-bootloader.bin)
- [yoradio-2p8-touch-public-partitions.bin](F:/My/YoRadio/2.8%20touch/yoradio-2p8-touch-public-partitions.bin)
- [yoradio-2p8-touch-public-boot_app0.bin](F:/My/YoRadio/2.8%20touch/yoradio-2p8-touch-public-boot_app0.bin)
- [yoradio-2p8-touch-public-firmware.bin](F:/My/YoRadio/2.8%20touch/yoradio-2p8-touch-public-firmware.bin)
- [yoradio-2p8-touch-public-spiffs.bin](F:/My/YoRadio/2.8%20touch/yoradio-2p8-touch-public-spiffs.bin)

## 12. Адреса для ручной прошивки бинарников

Если будешь шить через `esptool`, `Flash Download Tool` или другой флешер, адреса такие:

- `0x0000` -> `yoradio-2p8-touch-public-bootloader.bin`
- `0x8000` -> `yoradio-2p8-touch-public-partitions.bin`
- `0xE000` -> `yoradio-2p8-touch-public-boot_app0.bin`
- `0x10000` -> `yoradio-2p8-touch-public-firmware.bin`
- `0x310000` -> `yoradio-2p8-touch-public-spiffs.bin`

Это соответствует схеме разделов:

- `Huge APP (3MB No OTA/1MB SPIFFS)`

## 13. Если хочешь шить только из Arduino IDE

Тогда схема такая:

1. Код шьёшь кнопкой `Upload`
2. `SPIFFS` шьёшь отдельно, если он нужен

То есть `Arduino IDE` отлично подходит для обычной разработки, но надо помнить, что `SPIFFS` живёт отдельно от обычного скетча.

## 14. Простой сценарий обновления

Если ты поменял:

- `myoptions.h`
- `mytheme.h`
- `.cpp/.h` в проекте

то обычно достаточно:

1. `Verify`
2. `Upload`

Если ты поменял что-то внутри папки `data`, тогда после этого ещё надо перешить `SPIFFS`.

## 15. Если загрузка в плату не начинается

Попробуй по порядку:

1. закрой `Serial Monitor`
2. отключи USB
3. подключи USB заново
4. снова выбери `COM`-порт
5. нажми `Upload`

Если не помогло:

1. зажми кнопку `BOOT`
2. нажми `Upload`
3. когда прошивка реально стартует, кнопку `BOOT` можно отпустить

## 16. Если экран чёрный или с крокозябрами

Проверь:

- в `Tools` точно выбрана `Huge APP`
- `Flash Size` стоит `4MB`
- `PSRAM` стоит `OPI PSRAM`
- в [myoptions.h](F:/My/YoRadio/2.8%20touch/yoradio/yoRadio/myoptions.h) осталось:
  - `DSP_MODEL DSP_ILI9341`
  - `DSP_SPIPINS 12, 13, 11`
  - `DEFAULT_INVERTDISPLAY true`
  - `DEFAULT_FLIPSCREEN true`
- после полной очистки был прошит `SPIFFS`

## 17. Если не работает touch

Проверь, что в [myoptions.h](F:/My/YoRadio/2.8%20touch/yoradio/yoRadio/myoptions.h) стоят:

- `TS_MODEL TS_MODEL_FT6336`
- `TS_SDA 16`
- `TS_SCL 15`
- `TS_RST 18`
- `TS_INT 17`
- `TS_ADDR 0x38`

## 18. Если хочешь полностью чистую перепрошивку

Самый надёжный вариант:

1. В `Tools` временно поставь `Erase All Flash Before Sketch Upload -> Enabled`
2. Прошей скетч
3. Верни `Erase All Flash Before Sketch Upload -> Disabled`
4. После этого обязательно прошей `SPIFFS`

Без последнего шага система может стартовать криво или без части файлов.

## 19. Коротко: что делать дома

Если хочешь просто повторить рабочую прошивку:

1. Открываешь `yoRadio.ino`
2. Ставишь `ESP32S3 Dev Module`
3. Ставишь `Huge APP`
4. Ставишь `Flash Size 4MB`
5. Ставишь `PSRAM OPI PSRAM`
6. Нажимаешь `Verify`
7. Нажимаешь `Upload`
8. При чистой прошивке потом обязательно шьёшь `SPIFFS`

## 20. Самый короткий ответ

Да, прошивать этот проект через `Arduino IDE` правильно и удобно.

Для нашей платы главное не забыть четыре вещи:

- `ESP32S3 Dev Module`
- `Huge APP`
- `4MB flash`
- `SPIFFS` после полной очистки флеша
