# AGENTS.md

Instructions for a coding agent working in this repository. People should read
`README.md` instead.

## What this project is

A single Arduino sketch, `gabriel/`, for the Waveshare ESP32-S3-Touch-LCD-2.
It draws Gabriel on a 160×120 canvas (`lowres.h`), scales it 2× onto the
320×240 ST7789, and bites whenever the QMI8658 IMU detects a snap of the hand.

## Where changes go

| Request | File |
|---|---|
| Snap sensitivity, cooldown, brightness, rotation, intro, debug output | `gabriel/config.h` |
| How Gabriel looks or moves | `gabriel/gabriel.h` |
| How a snap is detected | `handSnapped()` in `gabriel/gabriel.ino` |
| Sensor / touch drivers | `gabriel/imu.h`, `gabriel/touch.h` |

User-facing changes go in `config.h`. Keep its comments: people with no C++
background read them to understand what they're changing.

## Commands

```bash
./tools/setup.sh      # install arduino-cli, ESP32 core, GFX library
./tools/build.sh      # compile only, no board required
./tools/flash.sh      # compile + upload
./tools/monitor.sh    # serial log at 115200
```

FQBN: `esp32:esp32:esp32s3:CDCOnBoot=cdc,FlashSize=16M,PSRAM=opi,PartitionScheme=huge_app`

## Rules

1. Run `./tools/build.sh` after every edit, and don't report success until it passes.
2. Only flash when the user asks you to. `flash.sh` writes to physical hardware.
3. Colors are RGB565, not RGB888.
