# yoRadio для ESP32-S3 ES3C28P 2.8" Touch

<p align="center">
  <img src="images/yologo.png" width="190" height="142" alt="yoRadio">
</p>

Готовая сборка интернет-радио для платы **LCDWIKI ES3C28P** с ESP32-S3, дисплеем 2.8", ёмкостным touch, microSD и вертикальным интерфейсом.

Этот README рассчитан на человека, который впервые увидел проект: ниже описано, что потребуется, как выбрать встроенный или внешний ЦАП, собрать проект в Arduino IDE, полностью прошить плату и выполнить первую настройку.

Проект основан на [оригинальном yoRadio](https://github.com/e2002/yoradio). В этой ветке используется официальная библиотека [ESP32-audioI2S Wolle 3.4.7](https://github.com/schreibfaul1/ESP32-audioI2S).

> [!IMPORTANT]
> Эта сборка требует **ESP32 Arduino Core 3.x** и **OPI PSRAM**. Проверенная версия Core — **3.3.10**. Core 2.x не подходит.

## Что поддерживается

- интернет-радиостанции;
- MP3, AAC/M4A, FLAC, OGG/Vorbis, Opus и WAV;
- воспроизведение файлов с microSD;
- дисплей ILI9341V 240x320 в вертикальной ориентации;
- ёмкостный touch FT6336G;
- встроенный кодек ES8311 или внешний I2S DAC;
- веб-интерфейс и редактор станций;
- погода, часы, формат и битрейт потока;
- VU-метр с отдельными текущими уровнями и пиками;
- настройка динамики VU из браузера;
- аудиозадача на core 0, интерфейс и основной loop на core 1;
- OPI PSRAM и framebuffer для плавного интерфейса;
- встроенный слот microSD по SD_MMC.

Анализатор `SPECTRUM` отключён, принудительный ресемплинг не используется.

## Какая плата нужна

Инструкция относится к:

```text
LCDWIKI ES3C28P
2.8" LCD Display ESP32-S3 240x320 Capacitive Touch
LCD: ILI9341V
Touch: FT6336G
```

Совместимая плата должна иметь ту же разводку GPIO. Для другой ESP32-S3 потребуется изменить [`yoRadio/myoptions.h`](yoRadio/myoptions.h).

## Распиновка

### Дисплей ILI9341V

| Сигнал | GPIO |
|---|---:|
| SCK | 12 |
| MISO | 13 |
| MOSI | 11 |
| CS | 10 |
| DC | 46 |
| RST | не используется |
| Подсветка | 45 |

### Touch FT6336G

| Сигнал | GPIO |
|---|---:|
| SDA | 16 |
| SCL | 15 |
| RST | 18 |
| INT | 17 |
| Адрес | `0x38` |

### Аудио I2S

| Сигнал | GPIO |
|---|---:|
| BCLK / BCK | 43 |
| LRC / WS | 44 |
| DOUT ESP32 → DIN DAC | 21 |
| MCLK | не используется |
| MUTE встроенного тракта | 1 |

### microSD, SD_MMC 4-bit

| Сигнал | GPIO |
|---|---:|
| CLK | 38 |
| CMD | 40 |
| D0 | 39 |
| D1 | 41 |
| D2 | 48 |
| D3 | 47 |

## Выбор аудиовыхода

Перед сборкой выберите **один** вариант.

### Встроенный на плату ES8311

У ESP32-S3 нет собственного аналогового DAC. Встроенным здесь называется отдельный кодек **ES8311**, установленный на плате ES3C28P.

Откройте [`yoRadio/myoptions.h`](yoRadio/myoptions.h) и добавьте:

```cpp
#define USE_ES8311 true
```

Проверьте, что ниже сохранены настройки:

```cpp
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

ES8311 и touch используют общую I2C-шину GPIO16/GPIO15 — для этой платы это нормально.

### Внешний I2S DAC или усилитель

Это режим ветки по умолчанию. Строка `USE_ES8311` должна отсутствовать или быть закомментирована:

```cpp
// #define USE_ES8311 true
```

Пины:

```cpp
#define I2S_MCLK 255
#define I2S_BCLK 43
#define I2S_LRC  44
#define I2S_DOUT 21
#define I2S_DIN  255
```

Подключение внешнего PCM5102A, MAX98357A или совместимого модуля:

| ESP32-S3 | I2S-модуль |
|---|---|
| GPIO43 | BCLK / BCK |
| GPIO44 | LRC / WS |
| GPIO21 | DIN |
| GND | GND |
| 3.3V или 5V | VIN строго по документации модуля |

Для PCM5102A и MAX98357A MCLK обычно не нужен.

Если GPIO1 не используется внешним усилителем для MUTE, установите:

```cpp
#define MUTE_PIN 255
```

Для монофонического усилителя можно добавить:

```cpp
#define PLAYER_FORCE_MONO true
```

> [!WARNING]
> Не включайте `I2S_INTERNAL`: внутреннего аналогового DAC у ESP32-S3 нет.

## Что установить на компьютер

### Arduino IDE

Установите Arduino IDE 2.x. Для описанного ниже SPIFFS uploader нужна версия не ниже 2.2.1.

### ESP32 Arduino Core

1. Откройте `File → Preferences`.
2. В `Additional boards manager URLs` добавьте:

```text
https://espressif.github.io/arduino-esp32/package_esp32_index.json
```

3. Откройте `Tools → Board → Boards Manager`.
4. Найдите `esp32 by Espressif Systems`.
5. Установите версию **3.3.10**.
6. Перезапустите Arduino IDE.

Можно использовать совместимую стабильную версию 3.x, но эта сборка проверена именно на 3.3.10.

### Библиотеки Arduino

Через `Tools → Manage Libraries` установите:

- Adafruit GFX Library;
- Adafruit BusIO;
- Adafruit ILI9341.

Проверенные версии:

| Библиотека | Версия |
|---|---:|
| Adafruit GFX Library | 1.12.6 |
| Adafruit BusIO | 1.17.4 |
| Adafruit ILI9341 | 1.6.3 |

ESP32-audioI2S устанавливать отдельно не нужно: версия 3.4.7 уже находится внутри проекта. Если в пользовательской папке Arduino есть другая библиотека с `Audio.h`, временно удалите или переименуйте её, чтобы избежать конфликта.

## Точные настройки Arduino IDE

Откройте файл [`yoRadio/yoRadio.ino`](yoRadio/yoRadio.ino). В меню `Tools` установите:

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
| Erase All Flash Before Sketch Upload | Disabled при обычном обновлении |

Самые важные пункты: **ESP32S3 Dev Module**, **Core 3.x**, **Huge APP** и **OPI PSRAM**.

## Зачем отдельно прошивать SPIFFS

Кнопка Upload записывает программу, но не веб-интерфейс и файлы из папки [`yoRadio/data`](yoRadio/data).

SPIFFS обязательно нужен:

- при первой прошивке;
- после полного стирания Flash;
- если менялись файлы в `yoRadio/data`;
- если отсутствует или сломан веб-интерфейс.

Для Arduino IDE 2.2.1+ установите [Arduino SPIFFS Upload](https://github.com/espx-cz/arduino-spiffs-upload):

1. скачайте файл `.vsix` из Releases;
2. закройте Arduino IDE;
3. создайте `%USERPROFILE%\.arduinoIDE\plugins`;
4. положите туда `.vsix`;
5. запустите Arduino IDE снова.

Команда загрузки вызывается через `Ctrl+Shift+P`:

```text
Upload SPIFFS to Pico/ESP8266/ESP32
```

Перед загрузкой SPIFFS закройте Serial Monitor.

## Полная прошивка новой или очищенной платы

### 1. Сохраните данные старой установки

Если радио уже работает, скачайте:

```text
http://IP-АДРЕС/data/playlist.csv
http://IP-АДРЕС/data/wifi.csv
```

Полное стирание удалит программу, SPIFFS, Wi-Fi, плейлист и сохранённые настройки.

### 2. Проверьте проект

1. Скачайте репозиторий или ветку `wolle-audio-3.4.7`.
2. Откройте `yoRadio/yoRadio.ino`.
3. Выберите встроенный ES8311 или внешний I2S DAC.
4. Выставьте параметры Arduino IDE из таблицы выше.
5. Подключите плату качественным USB-кабелем.
6. Выберите её COM-порт.

### 3. Прошейте программу

1. Для полностью чистой установки временно включите `Erase All Flash Before Sketch Upload → Enabled`.
2. Нажмите `Verify`.
3. Дождитесь успешной сборки.
4. Нажмите `Upload`.
5. Дождитесь окончания записи и перезапуска платы.
6. Сразу верните `Erase All Flash Before Sketch Upload → Disabled`.

### 4. Прошейте SPIFFS

1. Закройте Serial Monitor.
2. Не меняйте Board, Flash Size и Partition Scheme.
3. Нажмите `Ctrl+Shift+P`.
4. Выполните `Upload SPIFFS to Pico/ESP8266/ESP32`.
5. Дождитесь успешной записи.
6. Перезапустите плату.

Правильный порядок после полного стирания: **сначала программа, затем SPIFFS**.

### 5. Проверьте запуск

Откройте Serial Monitor на скорости:

```text
115200
```

В журнале должны определиться ESP32-S3, PSRAM, дисплей, I2S, SD и сеть. Объём PSRAM не должен быть равен нулю. При использовании ES8311 не должно появляться `ES8311 not found`.

## Первая настройка Wi-Fi

1. После запуска найдите сеть `yoRadioAP`.
2. Подключитесь с паролем `12345987`.
3. Откройте `http://192.168.4.1/`.
4. Выберите домашнюю Wi-Fi-сеть.
5. Введите её пароль и сохраните.
6. Дождитесь подключения радио.
7. IP-адрес появится на экране или в Serial Monitor.
8. Откройте `http://IP-АДРЕС/`.
9. Добавьте станции или импортируйте плейлист.

После обновления веб-интерфейса нажмите `Ctrl+F5` в браузере.

## Что прошивать при последующих изменениях

| Изменение | Действие |
|---|---|
| `.ino`, `.cpp`, `.h` | Upload программы |
| `myoptions.h` | Upload программы |
| `mytheme.h` | Upload программы |
| Файлы внутри `yoRadio/data` | Upload SPIFFS |
| Код и `data` | Сначала программа, затем SPIFFS |
| Изменилась схема разделов | Полное стирание, программа, SPIFFS |

При обычном обновлении этой же ветки оставляйте `Erase All Flash` выключенным.

## Веб-обновление

После первой USB-прошивки доступны:

- `http://IP-АДРЕС/update` — обновление готовыми BIN-файлами;
- `http://IP-АДРЕС/webboard` — загрузка отдельных веб-файлов;
- `http://IP-АДРЕС/emergency` — аварийная форма прошивки.

Firmware и SPIFFS — разные образы. Не загружайте один вместо другого.

## microSD

Слот настроен на SD_MMC в 4-битном режиме:

```cpp
#define SDMMC_INTERNAL true
#define SDMMC_1BIT     false
```

Рекомендуется карта FAT32. Не вынимайте её во время воспроизведения или сканирования. Поддерживаются MP3, AAC/M4A, FLAC, OGG/Vorbis, Opus и WAV.

## Настройка VU-метра

Все параметры доступны в веб-настройках и применяются без перепрошивки:

| Параметр | Диапазон | По умолчанию |
|---|---:|---:|
| Чувствительность | 25–300% | 100% |
| Окно измерения | 5–50 мс | 10 мс |
| Атака столбцов | 0–500 мс | 25 мс |
| Спад столбцов | 20–1500 мс | 180 мс |
| Удержание пиков | 0–1000 мс | 160 мс |
| Спад пиков | 100–3000 мс | 1800 мс |

Если VU слишком резкий, увеличьте атаку до 40–70 мс и спад до 220–350 мс. Если столбцы низкие, увеличьте чувствительность. Слишком большое окно уменьшает визуальную частоту обновления.

## Адреса готовых BIN-файлов

Для схемы `Huge APP (3MB No OTA/1MB SPIFFS)`:

| Образ | Адрес |
|---|---:|
| bootloader | `0x0000` |
| partitions | `0x8000` |
| boot_app0 | `0xE000` |
| firmware | `0x10000` |
| SPIFFS | `0x310000` |

BIN программы можно получить через `Sketch → Export Compiled Binary`. Не используйте SPIFFS-образ от другой схемы разделов.

## Если что-то не работает

### Не начинается Upload

1. Закройте Serial Monitor.
2. Переподключите USB.
3. Снова выберите COM-порт.
4. Уменьшите Upload Speed до 460800 или 115200.
5. Если нужно, зажмите BOOT, запустите Upload и отпустите BOOT после начала записи.

### Ошибка при сборке Audio

Проверьте:

- установлен ESP32 Core 3.x;
- выбрана ESP32S3 Dev Module;
- нет второй ESP32-audioI2S в пользовательской папке Arduino;
- открыт именно `yoRadio/yoRadio.ino`.

### Перезагрузки или проблемы с FLAC

Проверьте `PSRAM → OPI PSRAM`. Без PSRAM новые декодеры и framebuffer не смогут нормально работать.

### Чёрный экран

В `myoptions.h` должны быть:

```cpp
#define DSP_MODEL               DSP_ILI9341
#define DSP_SPIPINS             12, 13, 11
#define DEFAULT_INVERTDISPLAY   true
#define DEFAULT_FLIPSCREEN      true
#define ILI9341_PORTRAIT_LAYOUT true
```

Проверьте также GPIO10, GPIO46 и GPIO45.

### Не работает touch

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

Проверьте `USE_ES8311`, пины I2S `43/44/21`, I2C `16/15`, GPIO1 и журнал на наличие `ES8311 not found`.

### Нет звука с внешнего DAC

Проверьте, что `USE_ES8311` выключен, GPIO21 подключён к DIN, BCLK и LRC не перепутаны, земли соединены и модуль получает правильное питание. При необходимости установите `MUTE_PIN 255`.

### Нет веб-интерфейса

Прошейте SPIFFS из `yoRadio/data`, перезапустите плату и нажмите `Ctrl+F5`.

### После SPIFFS пропали Wi-Fi и станции

Полный SPIFFS-образ заменяет файловую систему. Восстановите резервные `wifi.csv` и `playlist.csv` через веб-интерфейс.

## Короткая последовательность

Для новой платы:

1. Arduino IDE 2.x;
2. ESP32 Core 3.3.10;
3. выбрать ES8311 или внешний DAC;
4. ESP32S3 Dev Module;
5. Huge APP;
6. OPI PSRAM;
7. Arduino и Events на Core 1;
8. Verify;
9. Upload программы;
10. выключить полное стирание;
11. Upload SPIFFS;
12. подключиться к `yoRadioAP`;
13. настроить Wi-Fi и станции.

## Дополнительная документация

- [Расширенная инструкция по Arduino IDE](yoRadio/ARDUINO_IDE_FLASHING_ES3C28P.md)
- [Карта платы и GPIO](yoRadio/BOARD_ES3C28P_2P8.md)
- [Настройка органов управления](Controls.md)
- [Галерея оригинального проекта](Images.md)
- [Оригинальный yoRadio](https://github.com/e2002/yoradio)
- [Официальная установка Arduino-ESP32](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html)
- [Описание меню Tools ESP32](https://docs.espressif.com/projects/arduino-esp32/en/latest/guides/tools_menu.html)
