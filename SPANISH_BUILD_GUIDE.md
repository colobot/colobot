# Colobot Spanish Translation - Build and Installation Guide

## Overview

This document describes the proper procedure for building and installing Colobot with Spanish translations, including common issues and their solutions.

## Critical Configuration Settings

### Required CMake Options

```bash
cmake -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/tmp/colobot-install \
    -DTRANSLATIONS=ON \
    -DCOLOBOT_INSTALL_DATA_DIR=/tmp/colobot-install/share/games/colobot
```

**IMPORTANT**: The `-DCOLOBOT_INSTALL_DATA_DIR` must be set explicitly to ensure all data (including translations) installs to the correct location.

## Installation Layout

After proper installation, the structure should be:

```
/tmp/colobot-install/
├── colobot                              # Main executable
├── lib/
│   └── colobot/
│       └── libCBot.so                   # Game library
├── share/
│   ├── applications/
│   │   └── colobot.desktop              # Desktop entry
│   ├── games/
│   │   └── colobot/                     # GAME DATA
│   │       ├── ai/
│   │       ├── fonts/
│   │       ├── help/
│   │       │   ├── E/                   # English help
│   │       │   ├── S/                   # Spanish help ✓
│   │       │   └── ...
│   │       ├── levels/
│   │       │   └── missions/
│   │       │       └── chapter001/
│   │       │           └── level001/
│   │       │               ├── help/
│   │       │               │   ├── help.S.txt    # Spanish ✓
│   │       │               │   └── soluce.S.txt  # Spanish ✓
│   │       │               └── scene.txt         # Contains Title.S
│   │       ├── models/
│   │       ├── music/
│   │       ├── sounds/
│   │       └── textures/
│   ├── locale/
│   │   └── es/
│   │       └── LC_MESSAGES/
│   │           └── colobot.mo           # UI translations ✓
│   └── man/
│       └── man6/
│           └── colobot.6
```

## Step-by-Step Build Procedure

### 1. Clean Environment

```bash
# Clean up previous builds
rm -rf /tmp/colobot-*
rm -rf build
```

### 2. Configure

```bash
cmake -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/tmp/colobot-install \
    -DTRANSLATIONS=ON \
    -DCOLOBOT_INSTALL_DATA_DIR=/tmp/colobot-install/share/games/colobot
```

### 3. Build

```bash
cd build
make -j$(nproc)
```

### 4. Install

```bash
make install
```

**Do NOT use `cmake --install .` as it may not properly trigger all translation targets.**

## Running the Game

### Correct Command

```bash
cd /tmp/colobot-install
LANGUAGE=es ./colobot -datadir /tmp/colobot-install/share/games/colobot
```

### WRONG Commands (Do NOT use)

```bash
# Don't use relative paths
./colobot -datadir ./data

# Don't use source directory
./colobot -datadir /home/user/colobot/data

# Don't forget the datadir parameter
./colobot
```

## Language Code Fix

### The Problem

Spanish language code was incorrectly set to 'E' (same as English), causing translation conflicts.

### The Fix

Modified `data/i18n-tools/scripts/common.py`:

```python
def language_char(self):
    if self.language == 'pt':
        return 'B'
    elif self.language == 'es':  # Added
        return 'S'                # Spanish now uses 'S'
    else:
        return self.language[0].upper()
```

### Result

- ✅ Spanish missions use: `Title.S`, `help.S.txt`, `soluce.S.txt`
- ✅ English uses: `Title.E`, `help.E.txt`, `soluce.E.txt`

## Common Issues and Solutions

### Issue 1: UI Not in Spanish

**Symptoms**: Menus and buttons show in English instead of Spanish

**Cause**: Missing or incorrect .mo file

**Solution**:
```bash
# Check if Spanish .mo exists
ls /tmp/colobot-install/share/locale/es/LC_MESSAGES/colobot.mo

# If missing, rebuild and reinstall
make -C build
make -C build install
```

### Issue 2: Mission Descriptions in English

**Symptoms**: Level titles and descriptions show in English

**Cause**: scene.txt not generated with Title.S

**Solution**:
```bash
# Regenerate translations
cd build
rm -rf data/levels/po
make
make install
```

### Issue 3: F1 Help in English

**Symptoms**: In-game help (F1) shows English text

**Cause**: help.S.txt not installed

**Solution**:
```bash
# Check if Spanish help exists
ls /tmp/colobot-install/share/games/colobot/levels/missions/chapter001/level001/help/help.S.txt

# If missing, reinstall
make -C build install
```

### Issue 4: Music/Sound Not Found

**Symptoms**: Game runs but no sound

**Cause**: Wrong data directory path

**Solution**: Always use full path:
```bash
LANGUAGE=es ./colobot -datadir /tmp/colobot-install/share/games/colobot
```

## Verification Checklist

After installation, verify:

- [ ] UI translations: `ls /tmp/colobot-install/share/locale/es/LC_MESSAGES/colobot.mo`
- [ ] Help translations: `ls /tmp/colobot-install/share/games/colobot/help/S/`
- [ ] Mission translations: `ls /tmp/colobot-install/share/games/colobot/levels/missions/chapter001/level001/help/help.S.txt`
- [ ] Spanish language code: `grep "Title.S" /tmp/colobot-install/share/games/colobot/levels/missions/chapter001/level001/scene.txt`

## Quick Test

```bash
# 1. Check UI translations exist
ls -lh /tmp/colobot-install/share/locale/es/LC_MESSAGES/colobot.mo

# 2. Check help translations exist
ls /tmp/colobot-install/share/games/colobot/help/S/cbot/ | head -5

# 3. Check mission translations exist
ls /tmp/colobot-install/share/games/colobot/levels/missions/chapter001/level001/help/help.S.txt

# 4. Run game
LANGUAGE=es /tmp/colobot-install/colobot -datadir /tmp/colobot-install/share/games/colobot
```

## Development Workflow

When updating translations:

1. Edit PO files in `data/` directory
2. Commit changes in data submodule
3. Push to fork
4. Update main repo submodule pointer
5. Clean rebuild:
   ```bash
   rm -rf build /tmp/colobot-*
   cmake -B build -DCMAKE_INSTALL_PREFIX=/tmp/colobot-install -DTRANSLATIONS=ON
   cd build && make -j$(nproc) && make install
   ```

## Summary

**Key Points**:
1. Always use `make install` not `cmake --install .`
2. Always specify full datadir path when running
3. Spanish uses 'S' language code (not 'E')
4. Data installs to `share/games/colobot/` not `data/`
5. UI translations are .mo files in `share/locale/es/`

**Working Configuration**:
- Install prefix: `/tmp/colobot-install`
- Data directory: `/tmp/colobot-install/share/games/colobot`
- Locale directory: `/tmp/colobot-install/share/locale`
- Run command: `LANGUAGE=es ./colobot -datadir /tmp/colobot-install/share/games/colobot`
