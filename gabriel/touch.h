/*
 * ==============================================================================
 *  touch.h  —  the capacitive touch panel (CST816), reduced to one question:
 *              "did someone just tap, and where?"
 * ==============================================================================
 *  The Waveshare ESP32-S3-Touch-LCD-2 carries a CST816 controller on I2C. It
 *  needs no configuration: five registers hold the finger count and its raw
 *  panel coordinates. This file wraps them and rotates the result to match
 *  SCREEN_ROTATION, so callers work in the same 320x240 space they draw in.
 *
 *  No touch panel (or a different one)? touchBegin() returns false and the
 *  sketch carries on without touch. That is a supported state, not an error.
 * ==============================================================================
 */

#pragma once

#include <Wire.h>

#define CST816_ADDR       0x15
#define CST816_REG_ID     0xA7
#define CST816_REG_NUM    0x02   // finger count; 0x03..0x06 are XH XL YH YL
#define CST816_CHIP_ID    0xB6

static bool touch_ready = false;

static bool touchRead(uint8_t reg, uint8_t *buf, uint8_t len) {
  Wire.beginTransmission(CST816_ADDR);
  Wire.write(reg);
  if (Wire.endTransmission(true) != 0) return false;
  if (Wire.requestFrom((uint8_t)CST816_ADDR, len) != len) return false;
  for (uint8_t i = 0; i < len; i++) buf[i] = Wire.read();
  return true;
}

static bool touchBegin() {
  Wire.begin(TOUCH_PIN_SDA, TOUCH_PIN_SCL);
  Wire.setClock(400000);

  uint8_t id = 0;
  for (int attempt = 0; attempt < 3; attempt++) {
    if (touchRead(CST816_REG_ID, &id, 1) && id == CST816_CHIP_ID) {
      touch_ready = true;
      return true;
    }
    delay(50);
  }
  touch_ready = false;
  return false;
}

// True while a finger is down, with its position in screen coordinates.
static bool touchGet(int *x, int *y) {
  if (!touch_ready) return false;

  uint8_t n = 0;
  if (!touchRead(CST816_REG_NUM, &n, 1) || n == 0) return false;

  uint8_t raw[4];
  if (!touchRead(CST816_REG_NUM + 1, raw, 4)) return false;
  int rx = ((raw[0] & 0x0F) << 8) | raw[1];   // native panel coordinates,
  int ry = ((raw[2] & 0x0F) << 8) | raw[3];   // i.e. before rotation

#if SCREEN_ROTATION == 1
  *x = ry;                 *y = SCREEN_H - 1 - rx;
#elif SCREEN_ROTATION == 3
  *x = SCREEN_W - 1 - ry;  *y = rx;
#elif SCREEN_ROTATION == 2
  *x = SCREEN_W - 1 - rx;  *y = SCREEN_H - 1 - ry;
#else
  *x = rx;                 *y = ry;
#endif
  return true;
}

// One tap = one event. Returns true once per finger-down, on release-free
// edges only, so holding a finger on the button doesn't retrigger.
static bool touchTapped(int *x, int *y) {
  static bool was_down = false;
  int tx, ty;
  bool down = touchGet(&tx, &ty);
  bool tapped = down && !was_down;
  was_down = down;
  if (tapped) { *x = tx; *y = ty; }
  return tapped;
}
