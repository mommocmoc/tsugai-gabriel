#!/usr/bin/env bash
#
# flash.sh — compile and upload to a connected board.
#
#   ./tools/flash.sh                     auto-detected port
#   ./tools/flash.sh /dev/cu.usbmodem101 use a specific port
#   GABRIEL_PORT=/dev/ttyUSB0 ./tools/flash.sh
#
# Then ./tools/monitor.sh to watch the serial log.
#
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"
need_cli

PORT="${1:-}"
if [ -z "$PORT" ]; then
  PORT="$(detect_port)" || die "No board found.
    - Is the USB cable plugged into the board's USB-C port?
    - Is it a data cable? Many charge-only cables look identical.
    - Try: arduino-cli board list
    - Still nothing? Hold BOOT, tap RESET, release BOOT, then re-run."
fi

bold "Flashing $SKETCH_NAME.ino to $PORT"
echo

arduino-cli compile \
  --fqbn "$FQBN" \
  --build-path "$BUILD_DIR" \
  --upload \
  --port "$PORT" \
  "$SKETCH"

echo
ok "Uploaded — Gabriel should appear within a second or two"
echo
echo "  Watch the log:  ./tools/monitor.sh"
