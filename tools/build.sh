#!/usr/bin/env bash
#
# build.sh — compile Gabriel. No board needed.
#
#   ./tools/build.sh              compile
#   ./tools/build.sh --clean      compile from scratch
#
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"
need_cli

case " $* " in *" --clean "*) info "clearing $BUILD_DIR"; rm -rf "$BUILD_DIR" ;; esac

bold "Compiling $SKETCH_NAME.ino"
info "board: $FQBN"
echo

arduino-cli compile \
  --fqbn "$FQBN" \
  --build-path "$BUILD_DIR" \
  --warnings default \
  "$SKETCH"

echo
ok "Build succeeded"
BIN="$BUILD_DIR/$SKETCH_NAME.ino.bin"
[ -f "$BIN" ] && info "firmware: $BIN ($(du -h "$BIN" | cut -f1))"
echo
echo "  Next: ./tools/flash.sh"
