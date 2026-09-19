#!/usr/bin/env bash
#
# monitor.sh — watch the serial output. Ctrl-C to quit.
#
# Useful when something looks wrong: the board reports display, touch and IMU init,
# and — with DEBUG_FORCE on — the strongest snap it felt.
#
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"
need_cli

PORT="${1:-}"
[ -z "$PORT" ] && { PORT="$(detect_port)" || die "No board found. Is it plugged in?"; }

bold "Monitoring $PORT at ${BAUD} baud — Ctrl-C to quit"
echo
arduino-cli monitor --port "$PORT" --config "baudrate=$BAUD"
