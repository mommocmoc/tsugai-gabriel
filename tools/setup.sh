#!/usr/bin/env bash
#
# setup.sh — one command to go from a blank machine to a buildable sketch.
#
# Installs arduino-cli (if missing), the ESP32 board core, and the graphics
# library. Safe to run more than once: everything here skips work already done.
#
#   ./tools/setup.sh
#
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

bold "Tsugai: Gabriel — environment setup"
echo

# ---------------------------------------------------------------- arduino-cli
if command -v arduino-cli >/dev/null 2>&1; then
  ok "arduino-cli $(arduino-cli version | awk '{print $3}')"
else
  info "installing arduino-cli..."
  if command -v brew >/dev/null 2>&1; then
    brew install arduino-cli
  else
    mkdir -p "$HOME/.local/bin"
    curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh \
      | BINDIR="$HOME/.local/bin" sh
    export PATH="$HOME/.local/bin:$PATH"
    warn "Installed to ~/.local/bin — add it to your PATH:"
    echo '      echo '"'"'export PATH="$HOME/.local/bin:$PATH"'"'"' >> ~/.zshrc'
  fi
  command -v arduino-cli >/dev/null 2>&1 || die "arduino-cli install failed"
  ok "arduino-cli installed"
fi

arduino-cli config init >/dev/null 2>&1 || true

# ------------------------------------------------------------------ ESP32 core
ESP32_URL="https://espressif.github.io/arduino-esp32/package_esp32_index.json"
info "registering the ESP32 board index..."
arduino-cli config add board_manager.additional_urls "$ESP32_URL" >/dev/null 2>&1 \
  || arduino-cli config set board_manager.additional_urls "$ESP32_URL" >/dev/null 2>&1
arduino-cli core update-index >/dev/null 2>&1

if arduino-cli core list 2>/dev/null | grep -q "^esp32:esp32"; then
  ok "ESP32 core $(arduino-cli core list | awk '/^esp32:esp32/{print $2}')"
else
  info "installing the ESP32 core (this pulls ~1GB of toolchain — go make coffee)"
  arduino-cli core install esp32:esp32
  ok "ESP32 core installed"
fi

# ------------------------------------------------------------------- libraries
# GFX : the ST7789 display driver and all drawing primitives
LIBS=("GFX Library for Arduino")
for lib in "${LIBS[@]}"; do
  if arduino-cli lib list 2>/dev/null | grep -qi "^${lib}[[:space:]]"; then
    ok "$lib"
  else
    info "installing $lib..."
    arduino-cli lib install "$lib" >/dev/null
    ok "$lib installed"
  fi
done

echo
bold "Ready."
echo
echo "  1. Edit  gabriel/config.h   — snap sensitivity, brightness"
echo "  2. Run   ./tools/build.sh   — check it compiles"
echo "  3. Run   ./tools/flash.sh   — plug in the board and send it"
echo
