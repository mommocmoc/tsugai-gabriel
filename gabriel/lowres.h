/*
 * ==============================================================================
 *  lowres.h  —  the 160x120 canvas Gabriel is drawn on
 * ==============================================================================
 *  Nothing here is drawn at the panel's real 320x240. Everything goes into a
 *  160x120 framebuffer and is pushed out as 2x2 blocks, so edges stay coarse
 *  and the whole thing looks like a game from 1993 rather than a smooth render.
 *
 *  Needs `gfx` to already exist — the sketch declares it before including this.
 * ==============================================================================
 */

#pragma once

#define LO_W 160
#define LO_H 120

static Arduino_Canvas *lo = nullptr;

// The framebuffer costs 38KB, so it is allocated the first time it is needed.
static bool loBegin() {
  if (!lo) {
    lo = new Arduino_Canvas(LO_W, LO_H, gfx);
    if (!lo->begin(GFX_SKIP_OUTPUT_BEGIN)) {
      Serial.println("[LO] low-res framebuffer allocation FAILED");
      delete lo;
      lo = nullptr;
      return false;
    }
  }
  return true;
}

// Push a low-res rectangle to the LCD, each pixel as a 2x2 block.
static void loBlit(int x, int y, int w, int h) {
  if (x < 0) { w += x; x = 0; }
  if (y < 0) { h += y; y = 0; }
  if (x + w > LO_W) w = LO_W - x;
  if (y + h > LO_H) h = LO_H - y;
  if (w <= 0 || h <= 0) return;

  static uint16_t buf[SCREEN_W * 8];
  const int rows_per_call = (SCREEN_W * 8) / (4 * w);
  uint16_t *fb = lo->getFramebuffer();
  while (h > 0) {
    int n = (h < rows_per_call) ? h : rows_per_call;
    uint16_t *dst = buf;
    for (int r = y; r < y + n; r++) {
      uint16_t *src = fb + r * LO_W + x;
      for (int c = 0; c < w; c++) {
        dst[0] = dst[1] = dst[2 * w] = dst[2 * w + 1] = src[c];
        dst += 2;
      }
      dst += 2 * w;
    }
    gfx->draw16bitRGBBitmap(x * 2, y * 2, buf, w * 2, n * 2);
    y += n;
    h -= n;
  }
}

// One animation frame: show what was drawn, then hold it for a moment.
static void loFrame() {
  loBlit(0, 0, LO_W, LO_H);
  delay(FRAME_MS);
}

static int loLerp(int a, int b, int t, int n) {
  if (n <= 0) return b;
  if (t > n) t = n;
  return a + (b - a) * t / n;
}

// Centered text with a hard black shadow, in the message line at the top.
static void loMsg(const char *s, uint16_t color) {
  int x = (LO_W - ((int)strlen(s) * 6 - 1)) / 2;
  lo->setTextSize(1);
  lo->setTextColor(0x0000);
  lo->setCursor(x + 1, 4); lo->print(s);
  lo->setTextColor(color);
  lo->setCursor(x, 3);     lo->print(s);
}

// A deterministic scratch of noise — same grain every run.
static uint32_t lo_seed = 1993;
static void loSeed(uint32_t s) { lo_seed = s; }
static uint8_t loRand() {
  lo_seed = lo_seed * 1103515245UL + 12345UL;
  return (lo_seed >> 16) & 0xFF;
}
