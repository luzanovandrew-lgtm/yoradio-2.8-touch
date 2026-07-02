#ifndef myoptions_h
#define myoptions_h

#define L10N_LANGUAGE     RU

#define DSP_MODEL         DSP_ILI9341
#define TFT_CS            10
#define TFT_DC            46
#define TFT_RST           -1
#define DSP_SPIPINS       12, 13, 11   /* SCK, MISO, MOSI */
#define BRIGHTNESS_PIN    45
#define DEFAULT_INVERTDISPLAY true
#define DEFAULT_FLIPSCREEN    true

#define TS_MODEL          TS_MODEL_FT6336
#define TS_SDA            16
#define TS_SCL            15
#define TS_RST            18
#define TS_INT            17
#define TS_ADDR           0x38

#define USE_ES8311
#define I2S_MCLK          4
#define I2S_BCLK          5
#define I2S_LRC           7
#define I2S_DOUT          8
#define I2S_DIN           6
#define ES8311_I2C_SDA    16
#define ES8311_I2C_SCL    15
#define ES8311_MAX_I2S    180
#define MUTE_PIN          1
#define MUTE_VAL          HIGH

#define ENC_BTNL          2
#define ENC_BTNB          3
#define ENC_BTNR          14
#define ENC_INTERNALPULLUP true

#define SDMMC_INTERNAL    true
#define SDMMC_1BIT        false
#define SDC_CLK           38
#define SDC_CMD           40
#define SDC_D0            39
#define SDC_D1            41
#define SDC_D2            48
#define SDC_D3            47

#endif
