/*
 * ==============================================================================
 *  gabriel.h  —  GABRIEL (가브리엘): two jaws, and far too many eyes
 * ==============================================================================
 *  Built from the reference: a pair of dark crescents with ragged, smoking
 *  edges, lined with heavy white teeth, and studded all over with ringed eyes —
 *  one big one in the middle of each jaw, the rest scattered around it.
 *
 *  The act:
 *    1  smoke gathers in an empty dark
 *    2  the two jaws slide in from the top and bottom, closed
 *    3  the eyes open, one after another, and look around
 *    4  the jaws yawn wide open — every eye fixes on you
 *    5  they snap shut, teeth meshing, white flash
 *    6  the whole thing comes apart into smoke; the last eye blinks out
 *
 *  Drawn on the low-res canvas from lowres.h.
 * ==============================================================================
 */

#pragma once

// ---- Palette ----------------------------------------------------------------
#define B_VOID      0x10A3   // #150d1a the dark it lives in
#define B_FLESH     0x0000   // the mass itself is pure black
#define B_EDGE      0x310A   // #341050 ragged rim
#define B_SMOKE     0x5936   // #5c28b0 wisps coming off it
#define B_EYE       0xFFFF   // white of the eye
#define B_EYE_RIM   0xA534   // #a7a7a7 shaded underside of the ring
#define B_PUPIL     0x0000   // black pupil
#define B_TOOTH     0xFFFF   // teeth
#define B_TOOTH_D   0x9CF3   // #9c9c9c tooth shadow
#define B_FLASH     0xFFFF

// ---- Where the eyes sit -----------------------------------------------------
// x, y as percentages of the jaw's half-width / thickness, plus a radius.
// The middle of each jaw carries the big one, as in the reference.
struct GabEye { int8_t x; int8_t y; int8_t r; };

static const GabEye GAB_EYES_UP[] = {
  {   0,  55, 5 }, { -34,  40, 4 }, {  34,  42, 4 }, { -62,  52, 3 }, {  62,  50, 3 },
  { -18,  78, 3 }, {  20,  76, 3 }, { -48,  76, 2 }, {  50,  74, 2 }, { -80,  66, 2 },
  {  80,  64, 2 }, { -12,  28, 2 }, {  14,  26, 2 }, { -70,  30, 2 }, {  72,  32, 2 },
};
static const GabEye GAB_EYES_LO[] = {
  {   0,  62, 4 }, { -30,  48, 3 }, {  32,  50, 3 }, { -58,  60, 2 }, {  58,  58, 2 },
  { -16,  84, 2 }, {  18,  82, 2 }, { -78,  44, 2 }, {  78,  46, 2 },
};
#define GAB_EYES_UP_N (sizeof(GAB_EYES_UP) / sizeof(GAB_EYES_UP[0]))
#define GAB_EYES_LO_N (sizeof(GAB_EYES_LO) / sizeof(GAB_EYES_LO[0]))

// Something to stare at, in SCREEN coordinates (320x240). While it is set,
// every eye turns its own way toward it and `look` is ignored; -1 means
// nothing to watch.
static int gab_tx = -1, gab_ty = -1;

// Every open eye drawn by the last gabrielPose(), so the stare can repaint
// just the eyes (see gabrielWatchEyes below).
#define GAB_EYES_MAX (GAB_EYES_UP_N + GAB_EYES_LO_N)
struct GabSpot { int16_t x, y, r; };          // canvas x runs past int8_t
static GabSpot gab_drawn[GAB_EYES_MAX];
static int    gab_drawn_n = 0;
static bool   gab_no_pupils = false;          // draw the whites only
static bool   gab_watch_ready = false;        // screen holds a pupil-less pose

static int gabPupilR(int r) { return (r < 3) ? 1 : r / 2; }

// One ringed eye: white disc, black pupil, a rim of shadow underneath.
static void gabEye(int x, int y, int r, int look, bool shut) {
  if (r < 1) return;
  if (shut) {                                  // closed: just a dark seam
    lo->fillRect(x - r, y, r * 2 + 1, 1, B_EYE_RIM);
    return;
  }
  lo->fillCircle(x, y + 1, r, B_EYE_RIM);
  lo->fillCircle(x, y, r, B_EYE);
  if (gab_drawn_n < GAB_EYES_MAX) gab_drawn[gab_drawn_n++] = { (int16_t)x, (int16_t)y, (int16_t)r };
  if (gab_no_pupils) return;

  int pr = gabPupilR(r);
  int reach = r - pr;                          // any further and it leaves the white
  if (reach < 0) reach = 0;

  int px = look, py = 0;
  if (gab_tx >= 0) {                           // staring at something
    int dx = gab_tx - x * 2, dy = gab_ty - y * 2;
    int len = max(abs(dx), abs(dy));
    px = (len > 0) ? (dx * reach) / len : 0;
    py = (len > 0) ? (dy * reach) / len : 0;
  }
  if (px >  reach) px =  reach;
  if (px < -reach) px = -reach;
  lo->fillCircle(x + px, y + py, pr, B_PUPIL);
}

/*
 *  One jaw.
 *    edge_y  the biting edge (teeth grow from it)
 *    dir     -1 for the upper jaw (mass sits above), +1 for the lower one
 *    thick   how deep the dark mass is
 *    grow    0..100, how much of the mass has formed (for arriving and leaving)
 *    look    pupil offset
 *    shut    eyes closed
 */
static void gabJaw(int edge_y, int dir, int thick, int grow, int look, bool shut) {
  const int cx = LO_W / 2;
  const int halfw = 76;
  if (grow <= 0) return;

  // --- the mass: a dome, thickest in the middle, ragged along the outside ---
  loSeed(dir < 0 ? 0xA17E : 0x5EED);
  for (int x = 0; x < LO_W; x++) {
    int dx = x - cx;
    int fall = (dx * dx * 100) / (halfw * halfw);         // 0 at center, 100 at tip
    if (fall > 100) continue;                             // past the point
    int depth = (thick * (100 - fall)) / 100;
    depth = (depth * grow) / 100;
    if (depth < 1) continue;

    uint8_t n = loRand();
    int ragged = depth - (n & 3);                          // chewed-up outer edge
    if ((n & 0x1F) == 0) ragged += 3;                      // the odd long wisp
    if (ragged < 1) continue;

    int y0 = (dir < 0) ? edge_y - ragged : edge_y + 1;
    lo->drawFastVLine(x, y0, ragged, B_FLESH);
    // the rim catches a little light, and smokes
    int rim = (dir < 0) ? y0 : y0 + ragged - 1;
    lo->drawPixel(x, rim, B_EDGE);
    lo->drawPixel(x, rim - dir, B_EDGE);
    if ((n & 0x07) == 0) lo->drawPixel(x, rim - dir * 2, B_SMOKE);
  }

  // --- teeth along the biting edge ------------------------------------------
  if (grow > 55) {
    const int reach = (halfw * 62) / 100;                  // the mouth, not the tips
    for (int x = cx - reach; x < cx + reach; x += 9) {
      int dx = x - cx + 4;
      int tall = 10 - (dx * dx * 6) / (reach * reach);     // longest in the middle
      int wide = 7 - (dx * dx * 2) / (reach * reach);      // and widest
      int ty = (dir < 0) ? edge_y - tall : edge_y + 1;
      lo->fillRect(x, ty, wide, tall, B_TOOTH);
      lo->drawFastVLine(x + wide - 1, ty, tall, B_TOOTH_D);
      lo->drawFastVLine(x - 1, ty, tall, B_FLESH);
    }
  }

  // --- and the eyes ---------------------------------------------------------
  if (grow > 70) {
    const GabEye *eyes = (dir < 0) ? GAB_EYES_UP : GAB_EYES_LO;
    int n = (dir < 0) ? GAB_EYES_UP_N : GAB_EYES_LO_N;
    for (int i = 0; i < n; i++) {
      int ex = cx + (eyes[i].x * halfw) / 100;
      int ey = edge_y + dir * ((eyes[i].y * thick) / 100);
      int edx = ex - cx;
      int efall = (edx * edx * 100) / (halfw * halfw);
      int edepth = ((thick * (100 - efall)) / 100) * grow / 100;
      if (efall > 100) continue;
      if (dir * (ey - edge_y) + eyes[i].r + 1 > edepth) continue;   // no room
      gabEye(ex, ey, eyes[i].r, look, shut);
    }
  }
}

// Centered text down in the gap between the jaws, where there is dark to put it.
static void gabGapMsg(const char *s, int y) {
  int x = (LO_W - ((int)strlen(s) * 6 - 1)) / 2;
  lo->setTextSize(1);
  lo->setTextColor(0x0000);
  lo->setCursor(x + 1, y + 1); lo->print(s);
  lo->setTextColor(B_EYE);
  lo->setCursor(x, y);         lo->print(s);
}

// Smoke in the dark: coarse specks that drift toward the middle.
static void gabSmoke(int density, int rise) {
  loSeed(0x6A6A + rise);
  for (int i = 0; i < density; i++) {
    int x = loRand() * LO_W / 256;
    int y = loRand() * LO_H / 256;
    int d = (y < LO_H / 2) ? rise : -rise;
    lo->fillRect(x, y + d, 2, 2, (i & 3) ? B_EDGE : B_SMOKE);
  }
}

// ---- Where the jaws sit ------------------------------------------------------
// Chosen so that the whole creature still fits on the 160x120 canvas at full
// gape: the upper mass reaches y = TOP_EDGE - MAX_GAP - THICK = 2, and the
// lower one ends at BOT_EDGE + MAX_GAP + THICK = 114. Widen the gape and the
// jaws start walking off the screen.
#define GAB_TOP_EDGE  56
#define GAB_BOT_EDGE  60
#define GAB_THICK     36
#define GAB_MAX_GAP   18

// One live frame, jaws held `gap` apart. Used by the tilt-driven sketch.
static void gabrielPose(int gap, int look, bool shut) {
  gab_drawn_n = 0;
  gab_watch_ready = false;
  lo->fillScreen(B_VOID);
  gabSmoke(30, 1);
  gabJaw(GAB_TOP_EDGE - gap, -1, GAB_THICK, 100, look, shut);
  gabJaw(GAB_BOT_EDGE + gap, +1, GAB_THICK, 100, look, shut);
}

// ---- The stare ---------------------------------------------------------------
// While a finger is on the glass the jaws and smoke don't move — only pupils
// do. So the scene is drawn once with empty whites, and after that each frame
// repaints just the eyes: every eye box is scaled up from the canvas and its
// pupil is drawn at the panel's full resolution, then only those boxes go out
// over SPI. That's a few hundred pixels a frame instead of 76,800.

// Draw the open pose once, whites only, and push the whole screen.
static void gabrielWatchBegin(int gap) {
  gab_no_pupils = true;
  gabrielPose(gap, 0, false);
  gab_no_pupils = false;
  loBlit(0, 0, LO_W, LO_H);
  gab_watch_ready = true;
}

// Repaint every eye's pupil toward (gab_tx, gab_ty).
static void gabrielWatchEyes() {
  if (!gab_watch_ready) return;
  static uint16_t buf[(2 * 5 + 4) * 2 * (2 * 5 + 5) * 2];
  uint16_t *fb = lo->getFramebuffer();

  for (int i = 0; i < gab_drawn_n; i++) {
    const GabSpot &e = gab_drawn[i];
    int x0 = e.x - e.r - 1, y0 = e.y - e.r - 1;       // canvas box, rim included
    int w = e.r * 2 + 3, h = e.r * 2 + 4;
    if (x0 < 0) { w += x0; x0 = 0; }
    if (y0 < 0) { h += y0; y0 = 0; }
    if (x0 + w > LO_W) w = LO_W - x0;
    if (y0 + h > LO_H) h = LO_H - y0;
    if (w <= 0 || h <= 0 || w * h * 4 > (int)(sizeof(buf) / sizeof(buf[0]))) continue;

    // the pupil, in panel pixels — the same size it is on the canvas, where a
    // radius-p circle spans 2p+1 canvas pixels, i.e. 4p+2 panel pixels
    float cx = e.x * 2 + 0.5f, cy = e.y * 2 + 0.5f;
    float pr = gabPupilR(e.r) * 2 + 0.5f;
    float reach = (e.r - gabPupilR(e.r)) * 2 + 0.5f;
    float dx = gab_tx - cx, dy = gab_ty - cy;
    float len = sqrtf(dx * dx + dy * dy);
    if (len > reach) { dx *= reach / len; dy *= reach / len; }
    float px = cx + dx, py = cy + dy;

    const int W = w * 2, H = h * 2;
    for (int yy = 0; yy < H; yy++) {
      const uint16_t *src = fb + (y0 + yy / 2) * LO_W + x0;
      float fy = (y0 * 2 + yy) - py;
      for (int xx = 0; xx < W; xx++) {
        uint16_t c = src[xx / 2];
        if (c == B_EYE) {                              // pupils never leave the white
          float fx = (x0 * 2 + xx) - px;
          if (fx * fx + fy * fy <= pr * pr) c = B_PUPIL;
        }
        buf[yy * W + xx] = c;
      }
    }
    gfx->draw16bitRGBBitmap(x0 * 2, y0 * 2, buf, W, H);
  }
}

// The bite, start to finish: he is waiting open, he takes the hand, the jaws
// slam shut, he holds it a moment — and then he opens again, waiting.
static void gabrielChomp(int idle_gap) {
  for (int f = 2; f >= 0; f--) {                  // down, fast
    gabrielPose((idle_gap * f) / 2, 0, false);
    loFrame();
  }
  lo->fillScreen(B_FLASH);                        // the moment it closes
  loFrame();
  for (int f = 0; f <= 4; f++) {                  // shut, chewing on it
    gabrielPose((f & 1) ? 2 : 0, 0, f > 2);
    loFrame();
  }
  for (int f = 0; f <= 4; f++) {                  // and open again
    gabrielPose((idle_gap * f) / 4, (f & 1) ? 1 : -1, false);
    loFrame();
  }
}

// ==============================================================================
//  The performance
// ==============================================================================
static void gabrielPlay() {
  if (!loBegin()) return;
  gab_watch_ready = false;

  const int top_edge_closed = GAB_TOP_EDGE, bot_edge_closed = GAB_BOT_EDGE;
  const int thick = GAB_THICK;

  // --- scene 1: smoke gathers ------------------------------------------------
  for (int f = 0; f <= 6; f++) {
    lo->fillScreen(B_VOID);
    gabSmoke(loLerp(20, 90, f, 6), f);
    loMsg("SOMETHING OPENS", B_EYE_RIM);
    loFrame();
  }

  // --- scene 2: the jaws arrive, shut ---------------------------------------
  for (int f = 0; f <= 8; f++) {
    lo->fillScreen(B_VOID);
    gabSmoke(60, 6 - f / 2);
    int grow = loLerp(10, 70, f, 8);
    gabJaw(loLerp(-20, top_edge_closed, f, 8), -1, thick, grow, 0, true);
    gabJaw(loLerp(LO_H + 20, bot_edge_closed, f, 8), +1, thick, grow, 0, true);
    loFrame();
  }

  // --- scene 3: the eyes open, and start looking ----------------------------
  for (int f = 0; f <= 9; f++) {
    lo->fillScreen(B_VOID);
    gabSmoke(40, 1);
    int look = (f < 4) ? 0 : ((f & 1) ? -1 : 1);
    gabJaw(top_edge_closed, -1, thick, loLerp(70, 100, f, 4), look, f < 2);
    gabJaw(bot_edge_closed, +1, thick, loLerp(70, 100, f, 4), look, f < 3);
    loMsg("GABRIEL", B_EYE);
    loFrame();
  }

  // --- scene 4: it yawns open ------------------------------------------------
  for (int f = 0; f <= 8; f++) {
    lo->fillScreen(B_VOID);
    gabSmoke(30, 1);
    int gap = loLerp(0, GAB_MAX_GAP, f, 8);
    gabJaw(top_edge_closed - gap, -1, thick, 100, 0, false);
    gabJaw(bot_edge_closed + gap, +1, thick, 100, 0, false);
    loFrame();
  }

  // --- scene 5: every eye on you, then the bite ------------------------------
  for (int f = 0; f <= 5; f++) {
    lo->fillScreen(B_VOID);
    int look = (f == 1) ? -2 : (f == 3) ? 2 : 0;
    gabJaw(top_edge_closed - GAB_MAX_GAP, -1, thick, 100, look, false);
    gabJaw(bot_edge_closed + GAB_MAX_GAP, +1, thick, 100, look, false);
    gabGapMsg("IT SEES YOU", LO_H / 2 - 3);
    loFrame();
  }
  for (int f = 0; f <= 2; f++) {
    lo->fillScreen(B_VOID);
    int gap = loLerp(GAB_MAX_GAP, 0, f, 2);
    gabJaw(top_edge_closed - gap, -1, thick, 100, 0, false);
    gabJaw(bot_edge_closed + gap, +1, thick, 100, 0, false);
    loFrame();
  }
  lo->fillScreen(B_FLASH);
  loFrame();

  // --- scene 6: it comes apart ----------------------------------------------
  for (int f = 0; f <= 9; f++) {
    lo->fillScreen(B_VOID);
    int grow = loLerp(100, 0, f, 9);
    gabJaw(top_edge_closed, -1, thick, grow, 0, f > 5);
    gabJaw(bot_edge_closed, +1, thick, grow, 0, f > 5);
    gabSmoke(loLerp(90, 20, f, 9), f);
    loFrame();
  }

  // --- curtain: one eye stays open a moment longer ---------------------------
  for (int f = 0; f <= 5; f++) {
    lo->fillScreen(B_VOID);
    gabEye(LO_W / 2, LO_H / 2, 5, 0, f > 3);
    loFrame();
  }
}
