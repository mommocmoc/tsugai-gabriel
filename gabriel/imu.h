/*
 * ==============================================================================
 *  imu.h  —  the motion sensor (QMI8658), reduced to "which way is down"
 * ==============================================================================
 *  The Waveshare board carries a QMI8658 on the same I2C bus as the touch
 *  panel. This file starts it and hands back the acceleration vector in g.
 *
 *  With the badge strapped to the back of a hand, that vector is gravity, and
 *  gravity moves when the wrist does — which is all the gesture needs.
 *
 *  No sensor? imuBegin() returns false and the sketch says so instead of
 *  pretending the hand is closed.
 * ==============================================================================
 */

#pragma once

#include <Wire.h>

#define QMI8658_ADDR       0x6B
#define QMI8658_WHO_AM_I   0x00
#define QMI8658_CHIP_ID    0x05
#define QMI8658_CTRL1      0x02
#define QMI8658_CTRL2      0x03
#define QMI8658_CTRL3      0x04
#define QMI8658_CTRL7      0x08
#define QMI8658_AX_L       0x35
#define QMI8658_RESET      0x60

// CTRL2 selects +/-4g, so this is what one count is worth.
#define QMI8658_ACC_LSB    (4.0f / 32768.0f)

static bool imu_ready = false;

static bool imuWrite(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(QMI8658_ADDR);
  Wire.write(reg);
  Wire.write(value);
  return Wire.endTransmission(true) == 0;
}

static bool imuRead(uint8_t reg, uint8_t *buf, uint8_t len) {
  Wire.beginTransmission(QMI8658_ADDR);
  Wire.write(reg);
  if (Wire.endTransmission(true) != 0) return false;
  if (Wire.requestFrom((uint8_t)QMI8658_ADDR, len) != len) return false;
  for (uint8_t i = 0; i < len; i++) buf[i] = Wire.read();
  return true;
}

static bool imuBegin() {
  Wire.begin(IMU_PIN_SDA, IMU_PIN_SCL);
  Wire.setClock(400000);

  uint8_t id = 0;
  for (int attempt = 0; attempt < 3; attempt++) {
    if (imuRead(QMI8658_WHO_AM_I, &id, 1) && id == QMI8658_CHIP_ID) break;
    delay(50);
  }
  if (id != QMI8658_CHIP_ID) { imu_ready = false; return false; }

  imuWrite(QMI8658_RESET, 0xB0);
  delay(15);
  imuWrite(QMI8658_CTRL1, 0x40);   // registers auto-increment on read
  imuWrite(QMI8658_CTRL7, 0x03);   // accelerometer + gyroscope on
  imuWrite(QMI8658_CTRL2, 0x95);   // accel  +/-4g   @ 250Hz
  imuWrite(QMI8658_CTRL3, 0xD5);   // gyro   512dps  @ 250Hz
  delay(20);

  imu_ready = true;
  return true;
}

// Acceleration in g. With the board held still this is gravity, pointing down.
static bool imuAccel(float *ax, float *ay, float *az) {
  if (!imu_ready) return false;
  uint8_t b[6];
  if (!imuRead(QMI8658_AX_L, b, 6)) return false;
  *ax = (int16_t)(b[0] | (b[1] << 8)) * QMI8658_ACC_LSB;
  *ay = (int16_t)(b[2] | (b[3] << 8)) * QMI8658_ACC_LSB;
  *az = (int16_t)(b[4] | (b[5] << 8)) * QMI8658_ACC_LSB;
  return true;
}
