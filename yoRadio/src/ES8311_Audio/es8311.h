#pragma once

#include <Arduino.h>
#include <Wire.h>

class ES8311 {
  public:
    explicit ES8311(TwoWire *wire = &Wire);

    bool begin(int32_t sda, int32_t scl, uint32_t frequency = 400000UL);
    bool setVolume(uint8_t volume);

  private:
    bool writeReg(uint8_t reg, uint8_t value);

    TwoWire *_wire;
};

extern ES8311 es;
