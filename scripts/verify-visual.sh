#!/bin/bash
# Launch colobot, wait for window, capture screenshot, then quit.
# Usage: ./scripts/verify-visual.sh [binary] [resolution] [output.png]
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

BINARY="${1:-$REPO_ROOT/build-dev/colobot}"
RESOLUTION="${2:-1600x1200}"
OUTPUT="${3:-/tmp/colobot-verify.png}"

if [ ! -x "$BINARY" ]; then
  echo "ERROR: binary not found or not executable: $BINARY" >&2
  exit 1
fi

BINARY_DIR="$(dirname "$BINARY")"
DATADIR="${COLOBOT_DATADIR:-$BINARY_DIR/data}"

echo "==> Launching $BINARY at $RESOLUTION (datadir: $DATADIR)..."
"$BINARY" -resolution "$RESOLUTION" -loglevel warn -datadir "$DATADIR" \
  -glversion 3.3 -glprofile core &
PID=$!

# Wait for the window to appear (poll up to 10s)
for i in $(seq 1 20); do
  sleep 0.5
  WIN=$(osascript -e "
    tell application \"System Events\"
      if exists (process whose unix id is $PID) then
        set p to first process whose unix id is $PID
        if (count of windows of p) > 0 then
          return id of first window of p
        end if
      end if
    end tell
    return \"\"" 2>/dev/null || echo "")
  if [ -n "$WIN" ]; then
    break
  fi
done

sleep 0.5  # let the first frame render

echo "==> Capturing screenshot (window id: ${WIN:-screen}) to $OUTPUT..."
if [ -n "$WIN" ]; then
  screencapture -x -l "$WIN" "$OUTPUT"
else
  screencapture -x "$OUTPUT"
fi

echo "==> Quitting game (pid $PID)..."
kill "$PID" 2>/dev/null || true
wait "$PID" 2>/dev/null || true

echo "==> Done. Screenshot saved to $OUTPUT"
