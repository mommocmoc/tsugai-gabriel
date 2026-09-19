#!/usr/bin/env bash
# Shared settings and helpers for the scripts in tools/.
# Not meant to be run directly — the other scripts source it.

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

SKETCH_NAME="gabriel"
SKETCH="$ROOT/$SKETCH_NAME"
BUILD_DIR="$ROOT/.build/$SKETCH_NAME"

# Waveshare ESP32-S3-Touch-LCD-2.
# 16MB flash, OPI PSRAM, USB-CDC serial, and a large app partition.
FQBN="esp32:esp32:esp32s3:CDCOnBoot=cdc,FlashSize=16M,PSRAM=opi,PartitionScheme=huge_app"

BAUD=115200

bold()  { printf "\033[1m%s\033[0m\n" "$*"; }
info()  { printf "  \033[36m%s\033[0m %s\n" "·" "$*"; }
ok()    { printf "  \033[32m%s\033[0m %s\n" "✓" "$*"; }
warn()  { printf "  \033[33m%s\033[0m %s\n" "!" "$*"; }
die()   { printf "  \033[31m%s\033[0m %s\n" "✗" "$*" >&2; exit 1; }

need_cli() {
  command -v arduino-cli >/dev/null 2>&1 \
    || die "arduino-cli not found. Run ./tools/setup.sh first."
}

# Find the board. Prefers a port arduino-cli has actually matched to an ESP32;
# falls back to the usual USB-serial device names.
detect_port() {
  if [ -n "${GABRIEL_PORT:-}" ]; then
    echo "$GABRIEL_PORT"; return 0
  fi

  local matched
  matched="$(arduino-cli board list 2>/dev/null \
    | awk '/esp32/ && $1 ~ /^\/dev\// {print $1; exit}')" || true
  if [ -n "$matched" ]; then echo "$matched"; return 0; fi

  local p
  for p in /dev/cu.usbmodem* /dev/cu.wchusbserial* /dev/cu.SLAB_USBtoUART* \
           /dev/ttyUSB* /dev/ttyACM*; do
    [ -e "$p" ] && { echo "$p"; return 0; }
  done
  return 1
}
