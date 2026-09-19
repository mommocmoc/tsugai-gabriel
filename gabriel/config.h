/*
 * ==============================================================================
 *  config.h  —  THE ONLY FILE YOU NEED TO EDIT
 * ==============================================================================
 *  GABRIEL, driven by your hand.
 *
 *  Strap the board to the back of your hand like a gauntlet. He waits there
 *  with his jaws open. Snap your fist shut and he bites down, holds it, and
 *  opens again — then he is waiting for the next one.
 *
 *  Build & flash:   ./tools/flash.sh
 * ==============================================================================
 */

#pragma once

// ==============================================================================
//  1. THE SNAP
// ==============================================================================
//  Gabriel waits with his jaws open. Snap your fist shut and he bites down,
//  holds it a moment, and opens again — the way he does in the show.
//
//  The sensor feels that snap as a jolt. This is how hard it has to be, in g:
//  0.30 goes off on a light tap, 0.60 wants a real snap.
#define SNAP_FORCE            0.45

// One snap should be one bite, so everything is ignored for this long after.
#define SNAP_COOLDOWN_MS       800

// How far apart the jaws wait. 100 is fully open.
#define IDLE_OPEN_PCT          100

// Play the full arrival — smoke, eyes opening, the stare — once at boot.
#define INTRO_ON_BOOT         true

// Tapping the screen bites too, which is handy for testing without a strap.
#define TOUCH_ENABLED         true

// Hold a finger on the screen and every eye turns to watch it.
#define EYES_FOLLOW_TOUCH     true

// A touch shorter than this, in milliseconds, counts as a tap and he bites.
// Hold it longer and he only watches.
#define TAP_BITE_MS            250

// Print the strongest jolt seen, twice a second, so you can pick SNAP_FORCE.
// Run ./tools/monitor.sh while you snap your hand, then set this to false.
#define DEBUG_FORCE           true


// ==============================================================================
//  2. SPEED
// ==============================================================================
// Pause between frames of the scripted parts, in milliseconds.
#define FRAME_MS                10


// ==============================================================================
//  3. SCREEN
// ==============================================================================
#define SCREEN_BRIGHTNESS       90    // 0-100
#define SCREEN_ROTATION          1    // 1 = landscape 320x240, 3 = flipped

#define SCREEN_W               320
#define SCREEN_H               240


// ==============================================================================
//  4. HARDWARE — don't touch unless you changed boards
// ==============================================================================
//  Waveshare ESP32-S3-Touch-LCD-2. The touch panel and the QMI8658 motion
//  sensor share one I2C bus.

#define LCD_PIN_SCLK   39
#define LCD_PIN_MOSI   38
#define LCD_PIN_MISO   40
#define LCD_PIN_DC     42
#define LCD_PIN_RST    -1
#define LCD_PIN_CS     45
#define LCD_PIN_BL      1

#define TOUCH_PIN_SDA  48
#define TOUCH_PIN_SCL  47
#define IMU_PIN_SDA    48
#define IMU_PIN_SCL    47

#define LCD_PANEL_W   240   // native panel width  (before rotation)
#define LCD_PANEL_H   320   // native panel height (before rotation)
