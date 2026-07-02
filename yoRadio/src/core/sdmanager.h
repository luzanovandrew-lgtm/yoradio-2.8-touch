#ifndef sdmanager_h
#define sdmanager_h

#include <Arduino.h>
#include <FS.h>

class SDManager {
  public:
    bool ready = false;
  public:
    bool start();
    void stop();
    bool cardPresent();
    File open(const char* path, const char* mode = FILE_READ, const bool create = false);
    bool exists(const char* path);
    bool remove(const char* path);
    FS* filesystem();
    void listSD(File &plSDfile, File &plSDindex, const char * dirname, uint8_t levels);
    void indexSDPlaylist();
  private:
    uint32_t _sdFCount = 0;
  private:
    bool _checkNoMedia(const char* path);
    bool _endsWith (const char* base, const char* str);
};

extern SDManager sdman;
#if defined(SD_SPIPINS) || SD_HSPI
extern SPIClass  SDSPI;
#endif
#endif
