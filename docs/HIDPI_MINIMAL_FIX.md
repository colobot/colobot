# HiDPI UI Scaling Fixes - Final Analysis

## Executive Summary

**Minimal Fix Required:** Only **2 changes** are strictly necessary:
1. **Font Texture Size** 256×256 → 512×512 (prevents text cutoff)
2. **GetNextTilePos() guard** (prevents SIGFPE crash)

All other changes are defensive improvements but not strictly required for basic HiDPI functionality.

## Critical vs. Defensive Changes

### MUST HAVE (Core Fix)

#### 1. Font Texture Size Increase
**File:** `text.cpp:155`
```cpp
constexpr glm::ivec2 FONT_TEXTURE_SIZE(512, 512);
```
**Why Required:**
- 256×256: ~4 chars at 120px (5× scale) - NOT ENOUGH
- 512×512: ~16 chars at 120px - SUFFICIENT
- Without this: Text gets cut off or doesn't render at large font sizes

#### 2. GetNextTilePos() Modulo Guard
**File:** `text.cpp:1396-1403`
```cpp
// Guard against tileSize larger than texture (would result in 0 tiles)
horizontalTiles = std::max(1, horizontalTiles);
verticalTiles = std::max(1, verticalTiles);
```
**Why Required:**
- Large fonts (120px at 5× scale) caused `tileSize > FONT_TEXTURE_SIZE`
- Made `horizontalTiles = 0`, leading to `tileNumber % 0` = SIGFPE
- **This was the actual crash cause**
- Without this: Game crashes with SIGFPE when opening SatCom on HiDPI

### NICE TO HAVE (Defensive Improvements)

The following guards prevent crashes in edge cases but may not be triggered in normal usage:

#### Slider Guards (6 locations in slider.cpp)
**Status:** Not strictly required - working without them
- Slider shows >100% on HiDPI but doesn't crash
- Visual indicator works even beyond 100%
- **Can be removed if minimal patch preferred**

#### Window Size Guards (engine.cpp)
**Locations:**
- `WindowToInterfaceCoords()` - guards m_size division
- `WindowToInterfaceSize()` - guards m_size division  
- `DrawStats()` - guards m_size division
- `SetFocus()` - guards aspect ratio

**Status:** Defensive - window size shouldn't be 0 in normal operation, but guards are safer

#### Edit Control Guards (edit.cpp)
**Locations:**
- `MoveAdjust()` - guards m_lineHeight
- `UpdateScroll()` - guards lineDiff

**Status:** Defensive - text editing should have valid dimensions

#### Map Guards (map.cpp)
**Locations:**
- `DetectObject()` - guards m_dim
- `AdjustOffset()` - guards m_zoom

**Status:** Defensive - map should be initialized with valid dimensions

### OPTIONAL (User Experience)

#### Font Scale Limit (5.0×)
**File:** `text.cpp:1258`
```cpp
scale = std::min(scale, 5.0f);
```
**Why Optional:**
- Prevents extreme fonts on 8K displays (would overflow 512×512)
- 4K displays (~4.4×) work fine at 5.0× limit
- Without limit: Might cause issues on extreme HiDPI displays

#### Mouse Cursor Scaling
**File:** `engine.cpp:4583`
```cpp
// Scale cursor based on window size
```
**Why Optional:**
- Cursor stays readable even at 32×32 on 4K
- Nice UX improvement but not crash-related

#### Slider Clamping
**File:** `displayinfo.cpp:428`
```cpp
// Clamp slider 0.0-1.0
```
**Why Optional:**
- Slider shows >100% without clamping, but works fine
- Visual quirk, not functional issue
- Text renders properly even with unclamped slider

## Minimal Patch (Production Ready)

```cpp
// text.cpp:155 - Texture size
constexpr glm::ivec2 FONT_TEXTURE_SIZE(512, 512);

// text.cpp:1396-1403 - GetNextTilePos guard
int horizontalTiles = FONT_TEXTURE_SIZE.x / std::max(1, fontTexture.tileSize.x);
int verticalTiles = FONT_TEXTURE_SIZE.y / std::max(1, fontTexture.tileSize.y);
// Guard against 0 tiles
horizontalTiles = std::max(1, horizontalTiles);
verticalTiles = std::max(1, verticalTiles);
```

**Total:** 8 lines of actual code changes
**Impact:** Fixes both text rendering and crashes

## Testing Results

**Configuration Tested:**
- 512×512 texture ✓
- GetNextTilePos guard ✓
- NO slider clamping ✓
- NO other division guards ✓

**Results:**
- ✅ No crashes
- ✅ Text renders fully
- ✅ Slider functional (shows >100% but works)
- ✅ Fonts readable on HiDPI

## Recommendation

**For Upstream PR:**

**Minimal Approach (Safe):**
Submit just the 2 critical fixes (texture size + GetNextTilePos). This is the smallest possible change that fixes the reported issue.

**Comprehensive Approach (Better):**
Include all guards as defensive programming. They don't hurt performance and make the code more robust against edge cases.

## Files Changed Analysis

| File | Lines | Type | Critical? |
|------|-------|------|-----------|
| text.cpp | 32 | Texture + GetNextTilePos | **YES** |
| engine.cpp | 68 | Cursor + 8 guards | Nice to have |
| slider.cpp | 26 | 6 slider guards | Nice to have |
| edit.cpp | 28 | 2 edit guards | Nice to have |
| map.cpp | 18 | 3 map guards | Nice to have |
| displayinfo.cpp | 11 | Slider clamping + cleanup | Nice to have |

## Conclusion

The crash and text rendering issues are resolved with just **2 core fixes**:
1. Larger texture (512×512) for scaled fonts
2. GetNextTilePos() guard preventing modulo by zero

All other changes are good defensive programming but the minimal functional fix requires only these 2 changes.
