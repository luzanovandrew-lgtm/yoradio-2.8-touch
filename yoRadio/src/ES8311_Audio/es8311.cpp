#include "es8311.h"

static constexpr uint8_t ES8311_ADDR = 0x18;

ES8311 es;

ES8311::ES8311(TwoWire *wire) : _wire(wire) {}

bool ES8311::begin(int32_t sda, int32_t scl, uint32_t frequency) {
    if (sda < 0 || scl < 0) {
        log_e("Invalid ES8311 SDA/SCL pins");
        return false;
    }

    if (!_wire->begin(sda, scl, frequency)) {
        log_e("ES8311 I2C begin failed");
        return false;
    }

    _wire->beginTransmission(ES8311_ADDR);
    if (_wire->endTransmission() != 0) {
        log_e("ES8311 not found");
        return false;
    }

    bool ok = true;
    ok &= writeReg(0x00, 0x1F);  // reset
    delay(20);
    ok &= writeReg(0x00, 0x00);  // release reset
    ok &= writeReg(0x00, 0x80);  // power on
    ok &= writeReg(0x01, 0x3F);  // enable clocks

    // 16-bit I2S, slave codec, ratios suitable for 256*Fs MCLK and 64*Fs BCLK
    ok &= writeReg(0x02, 0x00);
    ok &= writeReg(0x03, 0x10);
    ok &= writeReg(0x04, 0x10);
    ok &= writeReg(0x05, 0x00);
    ok &= writeReg(0x06, 0x00);
    ok &= writeReg(0x07, 0x00);
    ok &= writeReg(0x08, 0xFF);
    ok &= writeReg(0x09, 0x0C);
    ok &= writeReg(0x0A, 0x0C);

    ok &= writeReg(0x0D, 0x01);  // analog power
    ok &= writeReg(0x0E, 0x02);
    ok &= writeReg(0x12, 0x00);  // DAC power up
    ok &= writeReg(0x13, 0x10);  // output enable
    ok &= writeReg(0x1C, 0x6A);  // ADC EQ bypass / DC cancel
    ok &= writeReg(0x37, 0x08);  // DAC EQ bypass

    return ok;
}

bool ES8311::setVolume(uint8_t volume) {
    if (volume > 100) volume = 100;
    const uint8_t reg = (volume == 0) ? 0 : static_cast<uint8_t>((volume * 256 / 100) - 1);
    return writeReg(0x32, reg);
}

bool ES8311::writeReg(uint8_t reg, uint8_t value) {
    _wire->beginTransmission(ES8311_ADDR);
    _wire->write(reg);
    _wire->write(value);
    return _wire->endTransmission() == 0;
}
