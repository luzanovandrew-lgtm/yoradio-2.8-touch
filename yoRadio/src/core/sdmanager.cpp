#include "options.h"
#ifdef USE_SD
#include <Arduino.h>
#include "config.h"
#include "sdmanager.h"
#include "display.h"
#include "player.h"

#if SDMMC_INTERNAL
  #include <SD_MMC.h>
#else
  #include <SPI.h>
  #include <SD.h>
  #include "sd_diskio.h"
#endif

#if defined(SD_SPIPINS) || SD_HSPI
SPIClass  SDSPI(HSPI);
#define SDREALSPI SDSPI
#elif !SDMMC_INTERNAL
  #define SDREALSPI SPI
#endif

#ifndef SDSPISPEED
  #define SDSPISPEED 20000000
#endif

SDManager sdman;

FS* SDManager::filesystem() {
#if SDMMC_INTERNAL
  return &SD_MMC;
#else
  return &SD;
#endif
}

bool SDManager::start(){
#if SDMMC_INTERNAL
  if (SDMMC_1BIT) {
    SD_MMC.setPins(SDC_CLK, SDC_CMD, SDC_D0);
  } else {
    SD_MMC.setPins(SDC_CLK, SDC_CMD, SDC_D0, SDC_D1, SDC_D2, SDC_D3);
  }
  ready = SD_MMC.begin("/sdcard", SDMMC_1BIT, false);
  vTaskDelay(10);
  if(!ready) ready = SD_MMC.begin("/sdcard", SDMMC_1BIT, false);
  vTaskDelay(20);
  if(!ready) ready = SD_MMC.begin("/sdcard", SDMMC_1BIT, false);
  vTaskDelay(50);
  if(!ready) ready = SD_MMC.begin("/sdcard", SDMMC_1BIT, false);
#else
  ready = SD.begin(SDC_CS, SDREALSPI, SDSPISPEED);
  vTaskDelay(10);
  if(!ready) ready = SD.begin(SDC_CS, SDREALSPI, SDSPISPEED);
  vTaskDelay(20);
  if(!ready) ready = SD.begin(SDC_CS, SDREALSPI, SDSPISPEED);
  vTaskDelay(50);
  if(!ready) ready = SD.begin(SDC_CS, SDREALSPI, SDSPISPEED);
#endif
  return ready;
}

void SDManager::stop(){
#if SDMMC_INTERNAL
  SD_MMC.end();
#else
  SD.end();
#endif
  ready = false;
}

bool SDManager::cardPresent() {
  if(!ready) return false;
#if SDMMC_INTERNAL
  return SD_MMC.cardType() != CARD_NONE && SD_MMC.cardSize() > 0;
#else
  if(sectorSize()<1) {
    return false;
  }
  uint8_t buff[sectorSize()] = { 0 };
  bool bread = SD.readRAW(buff, 1);
  if(sectorSize()>0 && !bread) return false;
  return bread;
#endif
}

File SDManager::open(const char* path, const char* mode, const bool create) {
  return filesystem()->open(path, mode, create);
}

bool SDManager::exists(const char* path) {
  return filesystem()->exists(path);
}

bool SDManager::remove(const char* path) {
  return filesystem()->remove(path);
}

bool SDManager::_checkNoMedia(const char* path){
  if (path[strlen(path) - 1] == '/')
    snprintf(config.tmpBuf, sizeof(config.tmpBuf), "%s%s", path, ".nomedia");
  else
    snprintf(config.tmpBuf, sizeof(config.tmpBuf), "%s/%s", path, ".nomedia");
  bool nm = exists(config.tmpBuf);
  return nm;
}

bool SDManager::_endsWith (const char* base, const char* str) {
  int slen = strlen(str) - 1;
  const char *p = base + strlen(base) - 1;
  while(p > base && isspace(*p)) p--;
  p -= slen;
  if (p < base) return false;
  return (strncmp(p, str, slen) == 0);
}

void SDManager::listSD(File &plSDfile, File &plSDindex, const char* dirname, uint8_t levels) {
    File root = open(dirname);
    if (!root) {
        Serial.println("##[ERROR]#\tFailed to open directory");
        return;
    }
    if (!root.isDirectory()) {
        Serial.println("##[ERROR]#\tNot a directory");
        return;
    }

    uint32_t pos = 0;
    char* filePath;
    while (true) {
        vTaskDelay(2);
        player.loop();
        bool isDir;
        String fileName = root.getNextFileName(&isDir);
        if (fileName.isEmpty()) break;
        filePath = (char*)malloc(fileName.length() + 1);
        if (filePath == NULL) {
            Serial.println("Memory allocation failed");
            break;
        }
        strcpy(filePath, fileName.c_str());
        const char* fn = strrchr(filePath, '/') + 1;
        if (isDir) {
            if (levels && !_checkNoMedia(filePath)) {
                listSD(plSDfile, plSDindex, filePath, levels - 1);
            }
        } else {
            if (_endsWith(strlwr((char*)fn), ".mp3") || _endsWith(fn, ".m4a") || _endsWith(fn, ".aac") ||
                _endsWith(fn, ".wav") || _endsWith(fn, ".flac")) {
                pos = plSDfile.position();
                plSDfile.printf("%s\t%s\t0\n", fn, filePath);
                plSDindex.write((uint8_t*)&pos, 4);
                Serial.print(".");
                if(display.mode()==SDCHANGE) display.putRequest(SDFILEINDEX, _sdFCount+1);
                _sdFCount++;
                if (_sdFCount % 64 == 0) Serial.println();
            }
        }
        free(filePath);
    }
    root.close();
}

void SDManager::indexSDPlaylist() {
  _sdFCount = 0;
  if(exists(PLAYLIST_SD_PATH)) remove(PLAYLIST_SD_PATH);
  if(exists(INDEX_SD_PATH)) remove(INDEX_SD_PATH);
  File playlist = open(PLAYLIST_SD_PATH, "w", true);
  if (!playlist) {
    return;
  }
  File index = open(INDEX_SD_PATH, "w", true);
  listSD(playlist, index, "/", SD_MAX_LEVELS);
  index.flush();
  index.close();
  playlist.flush();
  playlist.close();
  Serial.println();
  delay(50);
}
#endif
