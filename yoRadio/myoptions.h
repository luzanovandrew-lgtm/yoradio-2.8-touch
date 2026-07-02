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

#define SDMMC_INTERNAL    true
#define SDMMC_1BIT        false
#define SDC_CLK           38
#define SDC_CMD           40
#define SDC_D0            39
#define SDC_D1            41
#define SDC_D2            48
#define SDC_D3            47

#endif
