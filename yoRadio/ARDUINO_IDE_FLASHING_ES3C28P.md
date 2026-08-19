# yoRadio для ESP32-S3 ES3C28P 2.8" Touch

Подробная инструкция по сборке, прошивке и первичной настройке этой ветки yoRadio через Arduino IDE.

Документ относится к плате `LCDWIKI ES3C28P` (или совместимой): ESP32-S3, дисплей ILI9341V 240x320, ёмкостный touch FT6336G, OPI PSRAM, встроенный слот microSD и вертикальный интерфейс yoRadio. В проект встроена официальная аудиобиблиотека Wolle ESP32-audioI2S 3.4.7.

> [!IMPORTANT]
> Для этой ветки обязателен ESP32 Arduino Core 3.x и режим `OPI PSRAM`. Проверенная версия Core — `3.3.10`. С Core 2.x ESP32-audioI2S 3.4.7 не собирается.

## Содержание

- [Возможности сборки](#возможности-сборки)
- [Аппаратная конфигурация](#аппаратная-конфигурация)
- [Выбор аудиовыхода](#выбор-аудиовыхода)
- [Подготовка Arduino IDE](#подготовка-arduino-ide)
- [Точные настройки платы](#точные-настройки-платы)
- [Первая прошивка с нуля](#первая-прошивка-с-нуля)
- [Первая настройка yoRadio](#первая-настройка-yoradio)
- [Обновление](#обновление)
- [microSD](#microsd)
- [Настройка VU-метра](#настройка-vu-метра)
- [BIN-файлы и адреса](#bin-файлы-и-адреса)
- [Диагностика](#диагностика)

## Возможности сборки

Ветка `wolle-audio-3.4.7` сохраняет возможности yoRadio и добавляет профиль конкретной ESP32-S3-платы:

- официальный аудиодвижок Wolle ESP32-audioI2S 3.4.7;
- MP3, AAC/M4A, FLAC, OGG/Vorbis, Opus и WAV;
- интернет-станции и файлы с microSD;
- ILI9341 в вертикальной ориентации и touch FT6336G;
- веб-интерфейс, редактор станций и настройки;
- отображение формата и битрейта;
- отдельные текущие уровни и пики VU-метра;
- настройка динамики VU из веб-интерфейса;
- аудиозадача на core 0;
- основной интерфейс и Arduino loop на core 1;
- отключённый `SPECTRUM`;
- исходная частота потока без принудительного ресемплинга.

Главные пользовательские файлы:

- [`myoptions.h`](myoptions.h) — плата, пины, дисплей, touch, аудио и SD;
- [`mytheme.h`](mytheme.h) — цвета интерфейса;
- [`BOARD_ES3C28P_2P8.md`](BOARD_ES3C28P_2P8.md) — краткая карта платы;
- [`data`](data) — веб-интерфейс, Wi-Fi и плейлист для SPIFFS.

Для настройки железа редактируйте `myoptions.h`, а не `src/core/options.h`.

## Аппаратная конфигурация

### Дисплей ILI9341V

| Сигнал | GPIO |
|---|---:|
| SCK | 12 |
| MISO | 13 |
| MOSI | 11 |
| CS | 10 |
| DC | 46 |
| RST | не используется (`-1`) |
| Подсветка | 45 |

### Touch FT6336G

| Сигнал | GPIO |
|---|---:|
| SDA | 16 |
| SCL | 15 |
| RST | 18 |
| INT | 17 |
| I2C-адрес | `0x38` |

### Аудио I2S

| Сигнал | GPIO |
|---|---:|
| BCLK | 43 |
| LRC / WS | 44 |
| DOUT ESP32 → DIN DAC | 21 |
| MCLK | не используется |
| MUTE встроенного тракта | 1 |

### Встроенный слот microSD (SD_MMC, 4-bit)

| Сигнал | GPIO |
|---|---:|
| CLK | 38 |
| CMD | 40 |
| D0 | 39 |
| D1 | 41 |
| D2 | 48 |
| D3 | 47 |

## Выбор аудиовыхода

### Встроенный на плату ES8311

У ESP32-S3 нет внутреннего аналогового DAC. Под «внутренним ЦАПом» здесь понимается отдельный кодек ES8311, установленный на самой плате.

Для ES8311 добавьте в `myoptions.h` флаг `USE_ES8311` и оставьте следующие настройки:

```cpp
#define USE_ES8311        true

#define I2S_MCLK          255
#define I2S_BCLK          43
#define I2S_LRC           44
#define I2S_DOUT          21
#define I2S_DIN           255

#define ES8311_I2C_SDA    16
#define ES8311_I2C_SCL    15
#define ES8311_MAX_I2S    180

#define MUTE_PIN          1
#define MUTE_VAL          HIGH
```

ES8311 и touch используют одну I2C-шину на GPIO16/GPIO15. Это предусмотрено профилем платы.

В режиме ES8311 пользовательская громкость регулируется самим кодеком. Не включайте одновременно режим ESP32 internal DAC.

### Внешний I2S DAC или I2S-усилитель

Это текущий режим ветки по умолчанию: `USE_ES8311` не определён. Подойдут, например, PCM5102A или MAX98357A.

```cpp
// #define USE_ES8311 true  // должно быть выключено

#define I2S_MCLK          255
#define I2S_BCLK          43
#define I2S_LRC           44
#define I2S_DOUT          21
#define I2S_DIN           255
```

Подключение:

| ESP32-S3 | Внешний модуль |
|---|---|
| GPIO43 | BCLK / BCK |
| GPIO44 | LRC / WS |
| GPIO21 | DIN |
| GND | GND |
| 3.3V или 5V | VIN — по документации модуля |

Для большинства PCM5102A и MAX98357A MCLK не требуется. Если GPIO1 не управляет вашим внешним усилителем, отключите MUTE:

```cpp
#define MUTE_PIN 255
```

Для монофонического усилителя можно включить:

```cpp
#define PLAYER_FORCE_MONO true
```

### Внутренний DAC самого ESP32

У ESP32-S3 нет встроенного аналогового DAC, который был у классического ESP32. Не включайте:

```cpp
#define I2S_INTERNAL true
```

Для этой платы используйте ES8311 либо внешний I2S-модуль.

### VS1053

Оригинальный yoRadio поддерживает VS1053, но профиль ES3C28P этой ветки протестирован с ESP32-audioI2S. Для VS1053 нужна отдельная SPI-разводка и проверка конфликтов с дисплеем и SD, поэтому это не рекомендуемый режим данной сборки.

## Подготовка Arduino IDE

### 1. Arduino IDE

Рекомендуется Arduino IDE 2.x. Для описанного ниже загрузчика SPIFFS нужна версия не ниже 2.2.1.

### 2. Пакет плат Espressif

В `File → Preferences → Additional boards manager URLs` добавьте официальный стабильный адрес:

```text
https://espressif.github.io/arduino-esp32/package_esp32_index.json
```

Затем:

1. откройте `Tools → Board → Boards Manager`;
2. найдите `esp32 by Espressif Systems`;
3. установите `3.3.10` или совместимую стабильную версию 3.x;
4. перезапустите Arduino IDE.

Версии 2.x для этой ветки не подходят.

### 3. Библиотеки

Через `Tools → Manage Libraries` установите:

- `Adafruit GFX Library`;
- `Adafruit BusIO`;
- `Adafruit ILI9341`.

Проверенные версии: Adafruit GFX 1.12.6, Adafruit BusIO 1.17.4 и Adafruit ILI9341 1.6.3.

ESP32-audioI2S отдельно устанавливать не нужно: версия 3.4.7 уже находится в `yoRadio/src/audioI2S`. Основные сетевые, touch- и сервисные зависимости также включены в проект.

Если IDE выбирает другую копию `Audio.h` из пользовательской папки libraries, удалите или временно переименуйте конфликтующую внешнюю ESP32-audioI2S.

### 4. SPIFFS uploader для Arduino IDE 2.x

Кнопка Upload записывает программу, но не содержимое папки `data`. Для первой установки нужен отдельный [SPIFFS uploader](https://github.com/espx-cz/arduino-spiffs-upload), совместимый с Arduino IDE 2.2.1+.

На Windows:

1. скачайте `.vsix` из Releases;
2. закройте Arduino IDE;
3. создайте `%USERPROFILE%\.arduinoIDE\plugins`, если папки нет;
4. скопируйте туда `.vsix`;
5. снова откройте Arduino IDE.

Для загрузки нажмите `Ctrl+Shift+P` и выберите:

```text
Upload SPIFFS to Pico/ESP8266/ESP32
```

Перед этим обязательно закройте Serial Monitor.

## Точные настройки платы

Откройте [`yoRadio.ino`](yoRadio.ino) и установите в меню `Tools`:

| Параметр | Значение |
|---|---|
| Board | ESP32S3 Dev Module |
| Port | COM-порт платы |
| USB Mode | Hardware CDC and JTAG |
| USB CDC On Boot | Disabled |
| Upload Mode | UART0 / Hardware CDC |
| CPU Frequency | 240MHz (WiFi) |
| Flash Mode | QIO 80MHz |
| Flash Size | 4MB (32Mb) |
| Partition Scheme | Huge APP (3MB No OTA/1MB SPIFFS) |
| PSRAM | OPI PSRAM |
| Arduino Runs On | Core 1 |
| Events Run On | Core 1 |
| Upload Speed | 921600 |
| Core Debug Level | None |
| Erase All Flash Before Sketch Upload | Disabled для обычного обновления |

Критически важны `ESP32S3 Dev Module`, Core 3.x, `Huge APP` и `OPI PSRAM`. Выбор Core 1 относится к Arduino loop и событиям; аудиодвижок сам запускает свою задачу на core 0.

## Первая прошивка с нуля

### Резервная копия

Если старое радио доступно по сети, заранее сохраните:

- `http://<IP-радио>/data/playlist.csv`;
- `http://<IP-радио>/data/wifi.csv`.

Полное стирание удаляет программу, SPIFFS, плейлист, Wi-Fi и настройки.

### Правильная последовательность

1. Подключите плату качественным USB-кабелем с передачей данных.
2. Откройте `yoRadio/yoRadio.ino`.
3. Выберите аудиорежим в `myoptions.h`.
4. Выставьте все параметры из таблицы выше.
5. Выберите COM-порт.
6. Для чистой установки временно включите `Erase All Flash Before Sketch Upload → Enabled`.
7. Нажмите `Verify` и дождитесь успешной сборки.
8. Нажмите `Upload` и дождитесь записи и перезапуска.
9. Сразу верните `Erase All Flash Before Sketch Upload → Disabled`.
10. Закройте Serial Monitor.
11. Не меняя плату и схему разделов, нажмите `Ctrl+Shift+P`.
12. Выполните `Upload SPIFFS to Pico/ESP8266/ESP32`.
13. Дождитесь окончания записи и перезапустите плату.
14. Откройте Serial Monitor на `115200` и проверьте старт.

После полного стирания порядок принципиален: сначала скетч, затем SPIFFS. Повторный Upload с включённым полным стиранием снова удалит файловую систему.

В журнале исправного запуска отображаются ESP32-S3, объём PSRAM, дисплей, I2S-пины, SD, сеть и player. В режиме ES8311 не должно быть сообщения `ES8311 not found`.

## Первая настройка yoRadio

Если подходящий Wi-Fi ещё не записан:

1. подключитесь к сети `yoRadioAP`;
2. пароль: `12345987`;
3. откройте `http://192.168.4.1/`;
4. выберите домашнюю сеть и введите пароль;
5. дождитесь подключения;
6. узнайте IP на экране или в Serial Monitor;
7. откройте `http://<IP-радио>/`;
8. добавьте станции или импортируйте плейлист.

После обновления веб-файлов нажмите `Ctrl+F5`, чтобы браузер не использовал старые CSS и JavaScript.

## Обновление

Перед крупным обновлением сохраните `playlist.csv` и `wifi.csv`.

| Что изменилось | Что прошивать |
|---|---|
| Только `.ino`, `.cpp`, `.h`, `myoptions.h` или `mytheme.h` | Только скетч |
| Файлы внутри `yoRadio/data` | SPIFFS |
| Код и `data` | Сначала скетч, затем SPIFFS |
| Схема разделов | Полное стирание, скетч, затем SPIFFS |
| Обычное обновление ветки | Не включать полное стирание |

Веб-адреса обслуживания:

- `http://<IP-радио>/update` — обновление бинарниками;
- `http://<IP-радио>/webboard` — отдельные файлы веб-интерфейса;
- `http://<IP-радио>/emergency` — аварийная форма прошивки.

Firmware и SPIFFS — разные образы и записываются в разные разделы.

## microSD

Встроенный слот работает через `SD_MMC` в 4-битном режиме:

```cpp
#define SDMMC_INTERNAL true
#define SDMMC_1BIT     false
```

Рекомендуется FAT32. Не вынимайте карту во время чтения. Поддерживаются MP3, AAC/M4A, FLAC, OGG/Vorbis, Opus и WAV; повреждённые или чрезмерно тяжёлые файлы могут не воспроизводиться.

## Настройка VU-метра

Параметры VU находятся в веб-настройках и применяются без перепрошивки:

| Настройка | Диапазон | По умолчанию | Назначение |
|---|---:|---:|---|
| Чувствительность | 25–300% | 100% | Высота столбцов |
| Окно измерения | 5–50 мс | 10 мс | Частота новых уровней |
| Атака столбцов | 0–500 мс | 25 мс | Движение вверх |
| Спад столбцов | 20–1500 мс | 180 мс | Движение вниз |
| Удержание пиков | 0–1000 мс | 160 мс | Пауза перед падением пика |
| Спад пиков | 100–3000 мс | 1800 мс | Скорость падения пиков |

Если VU слишком резкий, увеличьте атаку до 40–70 мс и спад до 220–350 мс. Если столбцы низкие, увеличьте чувствительность. Большое окно уменьшает визуальную частоту обновления.

## BIN-файлы и адреса

Arduino IDE создаёт бинарники через `Sketch → Export Compiled Binary`. Для `Huge APP (3MB No OTA/1MB SPIFFS)` используются:

| Образ | Адрес |
|---|---:|
| bootloader | `0x0000` |
| partitions | `0x8000` |
| boot_app0 | `0xE000` |
| firmware | `0x10000` |
| SPIFFS | `0x310000` |

Не записывайте SPIFFS от другой схемы разделов.

## Диагностика

### Прошивка не начинается

1. Закройте Serial Monitor.
2. Переподключите USB и снова выберите COM-порт.
3. Уменьшите Upload Speed до 460800 или 115200.
4. Зажмите BOOT, запустите Upload и отпустите BOOT после начала записи.
5. Проверьте USB-кабель.

### Ошибка компиляции Audio

Проверьте Core 3.x, плату ESP32S3 Dev Module и отсутствие второй ESP32-audioI2S в пользовательской папке Arduino libraries.

### Перезагрузки, нехватка памяти или нестабильный FLAC

Проверьте `PSRAM → OPI PSRAM`. В стартовом журнале объём PSRAM не должен быть равен нулю.

### Чёрный экран или неправильная ориентация

```cpp
#define DSP_MODEL               DSP_ILI9341
#define DSP_SPIPINS             12, 13, 11
#define DEFAULT_INVERTDISPLAY   true
#define DEFAULT_FLIPSCREEN      true
#define ILI9341_PORTRAIT_LAYOUT true
```

Проверьте также GPIO10, GPIO46 и GPIO45.

### Touch не работает или зеркален

```cpp
#define TS_MODEL          TS_MODEL_FT6336
#define TS_SDA            16
#define TS_SCL            15
#define TS_RST            18
#define TS_INT            17
#define TS_ADDR           0x38
#define DEFAULT_FLIPTOUCH true
```

### Нет звука со встроенного ES8311

Проверьте `USE_ES8311`, I2S `43/44/21`, I2C `16/15`, GPIO1 и отсутствие `ES8311 not found` в журнале.

### Нет звука с внешнего I2S DAC

Проверьте, что `USE_ES8311` выключен, GPIO21 идёт на DIN, BCLK и LRC не перепутаны, земли соединены, питание модуля правильное, а при необходимости `MUTE_PIN` равен `255`.

### Нет веб-интерфейса

Прошейте SPIFFS из `yoRadio/data`, перезагрузите плату и нажмите `Ctrl+F5`.

### После SPIFFS пропали станции или Wi-Fi

Полный образ SPIFFS заменяет файловую систему. Восстановите резервные `playlist.csv` и `wifi.csv` через веб-интерфейс либо поместите их в `yoRadio/data/data` до создания образа.

## Короткая памятка

Обычное обновление: Core 3.x → ESP32S3 Dev Module → Huge APP → OPI PSRAM → Core 1 → Erase All Flash выключен → Verify → Upload. SPIFFS нужен только при изменении `data`.

Чистая установка: резервная копия → однократное полное стирание → скетч → выключить стирание → SPIFFS → `yoRadioAP` → Wi-Fi и станции.

## Полезные ссылки

- [Официальная установка Arduino-ESP32](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html)
- [Параметры Tools для ESP32](https://docs.espressif.com/projects/arduino-esp32/en/latest/guides/tools_menu.html)
- [SPIFFS uploader для Arduino IDE 2.x](https://github.com/espx-cz/arduino-spiffs-upload)
- [Оригинальный yoRadio](https://github.com/e2002/yoradio)
- [Официальная ESP32-audioI2S](https://github.com/schreibfaul1/ESP32-audioI2S)
