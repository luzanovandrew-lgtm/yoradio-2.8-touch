#include "options.h"
#if (TS_MODEL!=TS_MODEL_UNDEFINED) && (DSP_MODEL!=DSP_DUMMY)
#include "Arduino.h"
#include "touchscreen.h"
#include "config.h"
#include "controls.h"
#include "display.h"
#include "player.h"
#include <Wire.h>

#ifndef TS_X_MIN
  #define TS_X_MIN              400
#endif
#ifndef TS_X_MAX
  #define TS_X_MAX              3800
#endif
#ifndef TS_Y_MIN
  #define TS_Y_MIN              260
#endif
#ifndef TS_Y_MAX
  #define TS_Y_MAX              3800
#endif
#ifndef TS_STEPS
  #define TS_STEPS              40
#endif
#ifndef TS_DOUBLE_TAP_MS
  #define TS_DOUBLE_TAP_MS      300
#endif
#ifndef TS_DOUBLE_TAP_MOVE
  #define TS_DOUBLE_TAP_MOVE    24
#endif

#if TS_MODEL==TS_MODEL_XPT2046
  #ifdef TS_SPIPINS
    SPIClass  TSSPI(HSPI);
  #endif
  #include <XPT2046_Touchscreen.h>
  XPT2046_Touchscreen ts(TS_CS);
#elif TS_MODEL==TS_MODEL_GT911
  #include "../GT911_Touchscreen/TAMC_GT911.h"
  TAMC_GT911 ts = TAMC_GT911(TS_SDA, TS_SCL, TS_INT, TS_RST, 0, 0);
#elif TS_MODEL==TS_MODEL_FT6336
  TwoWire& TSWire = Wire;
#endif

struct TouchPoint {
  uint16_t x;
  uint16_t y;
};

#if TS_MODEL==TS_MODEL_FT6336
static bool ft6336ReadBytes(uint8_t reg, uint8_t* data, size_t len) {
  TSWire.beginTransmission(TS_ADDR);
  TSWire.write(reg);
  if (TSWire.endTransmission(false) != 0) return false;
  size_t got = TSWire.requestFrom((uint8_t)TS_ADDR, (uint8_t)len);
  if (got != len) return false;
  for (size_t i = 0; i < len; ++i) data[i] = TSWire.read();
  return true;
}

static bool ft6336Touched() {
  uint8_t touches = 0;
  return ft6336ReadBytes(0x02, &touches, 1) && ((touches & 0x0F) > 0);
}

static bool ft6336ReadPoint(TouchPoint &point) {
  uint8_t data[4];
  if (!ft6336ReadBytes(0x03, data, sizeof(data))) return false;
  point.x = ((data[0] & 0x0F) << 8) | data[1];
  point.y = ((data[2] & 0x0F) << 8) | data[3];
  return true;
}

static TouchPoint ft6336RotatePoint(const TouchPoint &point, uint16_t width, uint16_t height, bool flipped) {
  TouchPoint out;
#if DSP_MODEL==DSP_ILI9341 && defined(ILI9341_PORTRAIT_LAYOUT) && ILI9341_PORTRAIT_LAYOUT
  if (flipped) {
    out.x = width - point.x;
    out.y = height - point.y;
  } else {
    out.x = point.x;
    out.y = point.y;
  }
#else
  if (flipped) {
    out.x = width - point.y;
    out.y = point.x;
  } else {
    out.x = point.y;
    out.y = height - point.x;
  }
#endif
  return out;
}
#endif

void TouchScreen::init(uint16_t w, uint16_t h){
  
#if TS_MODEL==TS_MODEL_XPT2046
  #ifdef TS_SPIPINS
    TSSPI.begin(TS_SPIPINS);
    ts.begin(TSSPI);
  #else
    #if TS_HSPI
      ts.begin(SPI2);
    #else
      ts.begin();
    #endif
  #endif
  ts.setRotation(config.store.fliptouch?3:1);
#endif
#if TS_MODEL==TS_MODEL_GT911
  ts.begin();
  ts.setRotation(config.store.fliptouch?0:2);
#endif
#if TS_MODEL==TS_MODEL_FT6336
  TSWire.begin(TS_SDA, TS_SCL);
  if (TS_RST != -1) {
    pinMode(TS_RST, OUTPUT);
    digitalWrite(TS_RST, LOW);
    delay(10);
    digitalWrite(TS_RST, HIGH);
    delay(50);
  }
  if (TS_INT != 255) {
    pinMode(TS_INT, INPUT);
  }
#endif
  _width  = w;
  _height = h;
#if TS_MODEL==TS_MODEL_GT911
  ts.setResolution(_width, _height);
#endif
}

tsDirection_e TouchScreen::_tsDirection(uint16_t x, uint16_t y) {
  int16_t dX = x - _oldTouchX;
  int16_t dY = y - _oldTouchY;
  if (abs(dX) > 20 || abs(dY) > 20) {
    if (abs(dX) > abs(dY)) {
      if (dX > 0) {
        return TSD_RIGHT;
      } else {
        return TSD_LEFT;
      }
    } else {
      if (dY > 0) {
        return TSD_DOWN;
      } else {
        return TSD_UP;
      }
    }
  } else {
    return TDS_REQUEST;
  }
}

void TouchScreen::flip(){
#if TS_MODEL==TS_MODEL_XPT2046
  ts.setRotation(config.store.fliptouch?3:1);
#endif
#if TS_MODEL==TS_MODEL_GT911
  ts.setRotation(config.store.fliptouch?0:2);
#endif
}

void TouchScreen::loop(){
  uint16_t touchX, touchY;
  uint16_t rawX = 0, rawY = 0;
  static bool wastouched = true;
  static uint32_t touchLongPress;
  static uint32_t pendingTapTicks = 0;
  static tsDirection_e direct;
  static uint16_t touchVol, touchStation;
  static uint16_t tapStartX, tapStartY;
  static uint16_t lastTouchX = 0, lastTouchY = 0;
  static uint16_t pendingTapX, pendingTapY;
  if (!wastouched && pendingTapTicks > 0 && (millis() - pendingTapTicks) > TS_DOUBLE_TAP_MS) {
    pendingTapTicks = 0;
    onBtnClick(EVT_BTNCENTER);
  }
  if (!_checklpdelay(20, _touchdelay)) return;
#if TS_MODEL==TS_MODEL_GT911
  ts.read();
#endif
  bool istouched = _istouched();
  if(istouched){
  #if TS_MODEL==TS_MODEL_XPT2046
    TS_Point p = ts.getPoint();
    rawX = p.x;
    rawY = p.y;
    touchX = map(rawX, TS_X_MIN, TS_X_MAX, 0, _width);
    touchY = map(rawY, TS_Y_MIN, TS_Y_MAX, 0, _height);
  #elif TS_MODEL==TS_MODEL_GT911
    rawX = ts.points[0].x;
    rawY = ts.points[0].y;
    touchX = rawX;
    touchY = rawY;
  #elif TS_MODEL==TS_MODEL_FT6336
    TouchPoint p;
    if (!ft6336ReadPoint(p)) {
      wastouched = false;
      return;
    }
    rawX = p.x;
    rawY = p.y;
    TouchPoint rotated = ft6336RotatePoint(p, _width, _height, config.store.fliptouch);
    touchX = rotated.x;
    touchY = rotated.y;
  #endif
    lastTouchX = touchX;
    lastTouchY = touchY;
  if (!wastouched) { /*     START TOUCH     */
      _oldTouchX = touchX;
      _oldTouchY = touchY;
      tapStartX = touchX;
      tapStartY = touchY;
      touchVol = touchX;
      touchStation = touchY;
      direct = TDS_REQUEST;
      touchLongPress=millis();
    } else { /*     SWIPE TOUCH     */
      direct = _tsDirection(touchX, touchY);
      switch (direct) {
        case TSD_LEFT:
        case TSD_RIGHT: {
            touchLongPress=millis();
            if(display.mode()==PLAYER || display.mode()==VOL){
              int16_t xDelta = map(abs(touchVol - touchX), 0, _width, 0, TS_STEPS);
              display.putRequest(NEWMODE, VOL);
              if (xDelta>1) {
                controlsEvent((touchVol - touchX)>0);
                touchVol = touchX;
              }
            }
            break;
          }
        case TSD_UP:
        case TSD_DOWN: {
            touchLongPress=millis();
            if(display.mode()==PLAYER || display.mode()==STATIONS){
              int16_t yDelta = map(abs(touchStation - touchY), 0, _height, 0, TS_STEPS);
              display.putRequest(NEWMODE, STATIONS);
              if (yDelta>1) {
                controlsEvent((touchStation - touchY)<0);
                touchStation = touchY;
              }
            }
            break;
          }
        default:
            break;
      }
    }
    if (config.store.dbgtouch) {
      Serial.print(", x = ");
      Serial.print(rawX);
      Serial.print(", y = ");
      Serial.println(rawY);
    }
  }else{
    if (wastouched) {/*     END TOUCH     */
      if (direct == TDS_REQUEST) {
        uint32_t pressTicks = millis()-touchLongPress;
        if( pressTicks < BTN_PRESS_TICKS*2){
          bool tapWithoutMove = abs((int)lastTouchX - (int)tapStartX) < TS_DOUBLE_TAP_MOVE &&
                                abs((int)lastTouchY - (int)tapStartY) < TS_DOUBLE_TAP_MOVE;
          if(pressTicks > 50 && tapWithoutMove) {
            bool isDoubleTap = pendingTapTicks > 0 &&
                               (millis() - pendingTapTicks) <= TS_DOUBLE_TAP_MS &&
                               abs((int)lastTouchX - (int)pendingTapX) < TS_DOUBLE_TAP_MOVE &&
                               abs((int)lastTouchY - (int)pendingTapY) < TS_DOUBLE_TAP_MOVE;
            if (isDoubleTap) {
              pendingTapTicks = 0;
              config.changeMode();
            } else {
              pendingTapTicks = millis();
              pendingTapX = lastTouchX;
              pendingTapY = lastTouchY;
            }
          } else {
            pendingTapTicks = 0;
          }
        }else{
          pendingTapTicks = 0;
          display.putRequest(NEWMODE, display.mode() == PLAYER ? STATIONS : PLAYER);
        }
      }
      direct = TSD_STAY;
    }
  }
  wastouched = istouched;
}

bool TouchScreen::_checklpdelay(int m, uint32_t &tstamp) {
  if (millis() - tstamp > m) {
    tstamp = millis();
    return true;
  } else {
    return false;
  }
}

bool TouchScreen::_istouched(){
#if TS_MODEL==TS_MODEL_XPT2046
  return ts.touched();
#elif TS_MODEL==TS_MODEL_GT911
  return ts.isTouched;
#elif TS_MODEL==TS_MODEL_FT6336
  return ft6336Touched();
#endif
}

#endif  // TS_MODEL!=TS_MODEL_UNDEFINED
