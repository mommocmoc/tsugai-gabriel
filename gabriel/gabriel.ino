/*
 * ==============================================================================
 *  GABRIEL  —  worn on the hand
 * ==============================================================================
 *  Board    : Waveshare ESP32-S3-Touch-LCD-2  (2.0" ST7789 IPS, 320x240)
 *  Shows    : Gabriel's two eye-studded jaws, waiting open. Strap the board to
 *             the back of your hand like a gauntlet.
 *
 *  Snap your fist shut and he bites: the jaws slam together, hold a moment,
 *  and open again. The QMI8658 feels that snap as a jolt — nothing else about
 *  how you hold your hand matters.
 *
 *  YOU PROBABLY DON'T NEED TO READ THIS FILE.  Everything tunable is in
 *  config.h; Gabriel himself is in gabriel.h.
 *
 *  Build & flash:   ./tools/flash.sh
 * ==============================================================================
 */

#include <Arduino_GFX_Library.h>

#include "config.h"

// ==============================================================================
//  Display
// ==============================================================================
Arduino_DataBus *bus = new Arduino_ESP32SPI(
    LCD_PIN_DC, LCD_PIN_CS, LCD_PIN_SCLK, LCD_PIN_MOSI, LCD_PIN_MISO);

Arduino_GFX *gfx = new Arduino_ST7789(
    bus, LCD_PIN_RST, SCREEN_ROTATION, true /* IPS */, LCD_PANEL_W, LCD_PANEL_H);

#include "lowres.h"
#include "gabriel.h"
#include "imu.h"

#if TOUCH_ENABLED
  #include "touch.h"
#endif

// ==============================================================================
//  The hand
// ==============================================================================
//  A snap is a jolt: either the acceleration stops looking like plain gravity,
//  or it changes direction all at once. Whichever is stronger is the signal, so
//  it doesn't matter which way round the board ends up on the hand.
static float fx = 0, fy = 0, fz = 1;      // smoothed acceleration
static float peak_force = 0;              // strongest jolt since the last print

static bool handSnapped() {
  float ax, ay, az;
  if (!imuAccel(&ax, &ay, &az)) return false;

  float dx = ax - fx, dy = ay - fy, dz = az - fz;
  float jerk = sqrtf(dx * dx + dy * dy + dz * dz);
  fx += dx * 0.25f;
  fy += dy * 0.25f;
  fz += dz * 0.25f;

  float mag = sqrtf(ax * ax + ay * ay + az * az);
  float heave = fabsf(mag - 1.0f);        // 1g is the hand just sitting there

  float force = (jerk > heave) ? jerk : heave;
  if (force > peak_force) peak_force = force;
  return force >= SNAP_FORCE;
}

// Waiting: jaws open, eyes wandering, the odd blink. `tick` keeps it alive.
static void drawIdle(int tick) {
  const int gap = (GAB_MAX_GAP * IDLE_OPEN_PCT) / 100;
  int look = (tick % 4 == 0) ? -2 : (tick % 4 == 2) ? 2 : 0;
  gabrielPose(gap, look, (tick % 9) == 5);
  loBlit(0, 0, LO_W, LO_H);
}

static void bite() {
  gabrielChomp((GAB_MAX_GAP * IDLE_OPEN_PCT) / 100);
}

static void message(const char *s) {
  if (!loBegin()) return;
  lo->fillScreen(B_VOID);
  loMsg(s, B_EYE);
  loBlit(0, 0, LO_W, LO_H);
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n=== Gabriel ===");

  ledcAttach(LCD_PIN_BL, 5000 /* Hz */, 10 /* bits */);
  ledcWrite(LCD_PIN_BL, (1 << 10) * SCREEN_BRIGHTNESS / 100);

  if (!gfx->begin()) {
    Serial.println("[LCD] init FAILED — check wiring / board selection");
  } else {
    Serial.printf("[LCD] ready (%dx%d)\n", gfx->width(), gfx->height());
  }
  gfx->fillScreen(0x0000);

#if TOUCH_ENABLED
  if (touchBegin()) Serial.println("[TOUCH] CST816 ready — tap to re-zero");
  else              Serial.println("[TOUCH] none");
#endif

  if (imuBegin()) Serial.println("[IMU] QMI8658 ready — snap to bite");
  else            Serial.println("[IMU] none — falling back to playing on its own");

#if INTRO_ON_BOOT
  gabrielPlay();
#endif
  drawIdle(0);
}

void loop() {
  static unsigned long last_bite = 0;
  static unsigned long last_idle = 0;
  static int tick = 0;

  // --- no motion sensor: perform on a tap, or on a timer --------------------
  if (!imu_ready) {
    static unsigned long last = 0;
#if TOUCH_ENABLED
    int tx, ty;
    if (touchTapped(&tx, &ty)) { bite(); return; }
    if (!touch_ready && millis() - last > 4000) { gabrielPlay(); last = millis(); }
#else
    if (millis() - last > 4000) { gabrielPlay(); last = millis(); }
#endif
    delay(20);
    return;
  }

  bool snap = handSnapped();

#if TOUCH_ENABLED
  int tx, ty;
  if (touchTapped(&tx, &ty)) snap = true;     // a tap bites too, for testing
#endif

  if (snap && millis() - last_bite >= SNAP_COOLDOWN_MS) {
    bite();
    last_bite = millis();
    last_idle = millis();
    return;
  }

  // Waiting, but not frozen: the eyes keep moving between bites.
  if (millis() - last_idle > 420) {
    last_idle = millis();
    drawIdle(++tick);
  }

#if DEBUG_FORCE
  static unsigned long last_print = 0;
  if (millis() - last_print > 500) {
    last_print = millis();
    Serial.printf("[SNAP] peak %.2f g (fires at %.2f)\n", peak_force, (float)SNAP_FORCE);
    peak_force = 0;
  }
#endif

  delay(8);
}
