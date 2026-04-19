# Verification Spec: Running and Interacting with Colobot

This document describes how Claude can build, launch, screenshot, and interact with the running Colobot game to verify visual changes (e.g. HiDPI font scaling, mouse cursor size, CJK character rendering).

---

## 1. Build Setup (one-time)

### 1a. Install dependencies (macOS/Homebrew)

```sh
brew install cmake sdl2 sdl2_image sdl2_ttf glew physfs flac libsndfile libvorbis vorbis-tools gettext libicns librsvg wget xmlstarlet glm
```

### 1b. Configure and build

```sh
cd /Users/ricardo/Documents/GitHub/colobot
mkdir -p build-dev && cd build-dev
cmake -DCMAKE_BUILD_TYPE=Debug -DCOLOBOT_DEVELOPMENT_MODE=ON -DTESTS=ON ..
make -j$(sysctl -n hw.logicalcpu)
```

The binary is at `build-dev/colobot`. In development mode, CMake copies `data/` into the build directory so `-datadir` is not required.

---

## 2. Launching the Game

### Standard windowed launch for verification

```sh
cd /Users/ricardo/Documents/GitHub/colobot/build-dev
./colobot -resolution 1600x1200 &
COLOBOT_PID=$!
sleep 3   # wait for SDL window to appear
```

Flags to know:
- `-resolution WxH` — sets window size (windowed, not fullscreen)
- `-datadir path` — override data directory (not needed with DEVELOPMENT_MODE)
- `-runscene sceneNNN` — jump directly into a level (skip menus), e.g. `-runscene scene001`
- `-loglevel debug` — verbose logging to stdout

### Kill the game when done

```sh
kill $COLOBOT_PID 2>/dev/null
```

---

## 3. Taking Screenshots

### Capture the entire screen

```sh
screencapture -x /tmp/colobot-screen.png
```

### Capture only the Colobot window

```sh
# Get the window ID of the colobot process
WINDOW_ID=$(osascript -e '
  tell application "System Events"
    set p to first process whose unix id is '"$COLOBOT_PID"'
    return id of first window of p
  end tell')
screencapture -x -l "$WINDOW_ID" /tmp/colobot-screen.png
```

Claude can then read `/tmp/colobot-screen.png` with the Read tool to visually inspect the output.

---

## 4. Sending Input to the Game

### Keyboard

```sh
# Activate the Colobot window, then send a key
osascript -e '
  tell application "System Events"
    tell process "colobot"
      set frontmost to true
      key code 53  -- Escape key
    end tell
  end tell'
```

Common key codes: `53` = Escape, `36` = Return/Enter, `49` = Space.

### Mouse click at a coordinate (relative to screen)

```sh
cliclick c:800,600    # click at screen coords 800,600
```

`cliclick` must be installed: `brew install cliclick`.

---

## 5. Verification Scenarios

### 5a. Font size on HiDPI display

1. Launch at `1600x1200` (2× reference resolution `800x600`)
2. Screenshot the main menu
3. Visually check that UI text is legible and proportional (font point size should be ~`size * sqrt(2)` larger than on 800×600)

### 5b. Mouse cursor size

1. Launch at high resolution
2. Screenshot — confirm the cursor is visible at normal size (the SDL_SetWindowSize fix)

### 5c. CJK characters (future)

1. Switch game language to Chinese/Japanese/Korean via settings or `-langdir`
2. Navigate to any screen with body text
3. Screenshot and verify glyphs render (not empty boxes)

---

## 6. Full Verification One-Liner Script

Save as `scripts/verify-visual.sh`:

```sh
#!/bin/bash
set -e

BINARY="${1:-./colobot}"
RESOLUTION="${2:-1600x1200}"
OUTPUT="${3:-/tmp/colobot-verify.png}"

echo "Launching $BINARY at $RESOLUTION..."
"$BINARY" -resolution "$RESOLUTION" &
PID=$!
sleep 4

echo "Capturing screenshot to $OUTPUT..."
WINDOW_ID=$(osascript -e "
  tell application \"System Events\"
    set p to first process whose unix id is $PID
    return id of first window of p
  end tell" 2>/dev/null || echo "")

if [ -n "$WINDOW_ID" ]; then
  screencapture -x -l "$WINDOW_ID" "$OUTPUT"
else
  screencapture -x "$OUTPUT"
fi

kill "$PID" 2>/dev/null
echo "Done. Screenshot at $OUTPUT"
```

Usage:
```sh
chmod +x scripts/verify-visual.sh
./scripts/verify-visual.sh build-dev/colobot 1600x1200 /tmp/verify.png
```
