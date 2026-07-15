/*************************************************************************************
    Custom ILI9341 portrait 240x320 layout for the LCDWIKI ES3C28P / ESP32-S3 2.8" touch board.
    Board profile summary:
      - LCD: ILI9341V, 320x240, SPI on GPIO12/13/11 with CS=10 DC=46 BL=45
      - Touch: FT6336G, I2C on GPIO16/15 with RST=18 INT=17
      - Storage: onboard SD_MMC slot on GPIO38/40/39/41/48/47
      - Audio: this project keeps board pin choices in myoptions.h
      - Theme: LCD palette is defined in mytheme.h
    Based on upstream displayILI9341conf.h with a centered composition tailored for this panel.
*************************************************************************************/

#ifndef displayILI9341conf_custom_h
#define displayILI9341conf_custom_h

#define DSP_WIDTH       240
#define TFT_FRAMEWDT    6
#define MAX_WIDTH       DSP_WIDTH-TFT_FRAMEWDT*2

#define TITLE_FIX       0
#define bootLogoTop     100

/* SROLLS  */                            /* {{ left, top, fontsize, align }, buffsize, uppercase, width, scrolldelay, scrolldelta, scrolltime } */
const ScrollConfig metaConf       PROGMEM = {{ TFT_FRAMEWDT, TFT_FRAMEWDT + 2, 2, WA_LEFT }, 140, false, MAX_WIDTH, 5000, 4, 30 };
const ScrollConfig title1Conf     PROGMEM = {{ TFT_FRAMEWDT, 38, 2, WA_LEFT }, 140, false, MAX_WIDTH-TITLE_FIX, 5000, 4, 30 };
const ScrollConfig title2Conf     PROGMEM = {{ TFT_FRAMEWDT, 60, 2, WA_LEFT }, 140, false, MAX_WIDTH-TITLE_FIX, 5000, 4, 30 };
const ScrollConfig playlistConf   PROGMEM = {{ TFT_FRAMEWDT, 106, 2, WA_LEFT }, 140, false, MAX_WIDTH, 1000, 4, 30 };
const ScrollConfig apTitleConf    PROGMEM = {{ 0, TFT_FRAMEWDT, 3, WA_CENTER }, 140, false, DSP_WIDTH, 0, 4, 20 };
const ScrollConfig apSettConf     PROGMEM = {{ TFT_FRAMEWDT, 320-TFT_FRAMEWDT-16, 2, WA_LEFT }, 140, false, MAX_WIDTH, 0, 4, 30 };
const ScrollConfig weatherConf    PROGMEM = {{ 100, 106, 2, WA_LEFT }, 180, false, 132, 0, 4, 30 };

/* BACKGROUNDS  */                       /* {{ left, top, fontsize, align }, width, height, outlined } */
const FillConfig   metaBGConf     PROGMEM = {{ 0, 0, 0, WA_LEFT }, DSP_WIDTH, 28, false };
const FillConfig   metaBGConfInv  PROGMEM = {{ 0, 28, 0, WA_LEFT }, DSP_WIDTH, 1, false };
const FillConfig   title2LineConf PROGMEM = {{ TFT_FRAMEWDT, 86, 0, WA_LEFT }, MAX_WIDTH, 1, false };
const FillConfig   volbarConf     PROGMEM = {{ TFT_FRAMEWDT, 312, 0, WA_LEFT }, MAX_WIDTH, 5, true };
const FillConfig  playlBGConf     PROGMEM = {{ 0, 101, 0, WA_LEFT }, DSP_WIDTH, 24, false };
const FillConfig  heapbarConf     PROGMEM = {{ 0, 318, 0, WA_LEFT }, DSP_WIDTH, 1, false };

/* WIDGETS  */                           /* { left, top, fontsize, align } */
const WidgetConfig bootstrConf    PROGMEM = { 0, 214, 1, WA_CENTER };
const WidgetConfig bitrateConf    PROGMEM = { 186, 102, 1, WA_LEFT };
const WidgetConfig voltxtConf     PROGMEM = { 0, 301, 1, WA_CENTER };
const WidgetConfig  iptxtConf     PROGMEM = { TFT_FRAMEWDT, 301, 1, WA_LEFT };
const WidgetConfig   rssiConf     PROGMEM = { TFT_FRAMEWDT, 296, 2, WA_RIGHT };
const WidgetConfig numConf        PROGMEM = { 0, 196, 0, WA_CENTER };
const WidgetConfig apNameConf     PROGMEM = { TFT_FRAMEWDT, 66, 2, WA_CENTER };
const WidgetConfig apName2Conf    PROGMEM = { TFT_FRAMEWDT, 90, 2, WA_CENTER };
const WidgetConfig apPassConf     PROGMEM = { TFT_FRAMEWDT, 130, 2, WA_CENTER };
const WidgetConfig apPass2Conf    PROGMEM = { TFT_FRAMEWDT, 154, 2, WA_CENTER };
const WidgetConfig  clockConf     PROGMEM = { 0, 245, 0, WA_CENTER };
const WidgetConfig vuConf         PROGMEM = { 4, 268, 1, WA_LEFT };

const WidgetConfig bootWdtConf    PROGMEM = { 0, 194, 1, WA_CENTER };
const ProgressConfig bootPrgConf  PROGMEM = { 90, 14, 4 };
const BitrateConfig fullbitrateConf PROGMEM = {{ 188, 94, 2, WA_LEFT }, 46 };

/* BANDS  */                             /* { onebandwidth, onebandheight, bandsHspace, bandsVspace, numofbands, fadespeed } */
const VUBandsConfig bandsConf     PROGMEM = { 232, 9, 2, 1, 24, 3 };

/* STRINGS  */
const char         numtxtFmt[]    PROGMEM = "%d";
const char           rssiFmt[]    PROGMEM = "WiFi %d";
const char          iptxtFmt[]    PROGMEM = "\010 %s";
const char         voltxtFmt[]    PROGMEM = "\023\025%d";
const char        bitrateFmt[]    PROGMEM = "%d kBs";

/* MOVES  */                             /* { left, top, width } */
const MoveConfig    clockMove     PROGMEM = { 0, 245, -1 };
const MoveConfig   weatherMove    PROGMEM = { 100, 106, 132 };
const MoveConfig   weatherMoveVU  PROGMEM = { 100, 106, 132 };

#endif
