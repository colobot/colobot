# HiDPI Font Scaling Issue - Investigation Notes

## Problem Description

Font size in help displays becomes very small on HiDPI displays (e.g., 4K monitors at 200% scaling). The issue manifests differently depending on context:

1. **Mission Help (F1)**: Font appears "normal" initially but becomes tiny when changing font size
2. **CBOT Help from UI**: Font is already tiny when opened without entering a mission first

## Root Cause Analysis

### Font Calculation Formula

The font size is calculated in two locations:

**1. CDisplayInfo::ViewDisplayInfo()** (colobot-base/src/ui/displayinfo.cpp:899)
```cpp
void CDisplayInfo::ViewDisplayInfo()
{
    auto dim = m_engine->GetWindowSize();
    edit->SetFontSize(CSettings::GetInstancePointer()->GetFontSize()/(dim.x / 640.0f));
}
```

**2. CStudio::ViewEditScript()** (colobot-base/src/ui/studio.cpp:1117)
```cpp
void CStudio::ViewEditScript()
{
    glm::ivec2 dim = m_engine->GetWindowSize();
    edit->SetFontSize(m_settings->GetFontSize()/(dim.x/640.0f));
}
```

Both use: `FontSize / (WindowWidth / 640)`

### The Problem

On a 4K display (3840px width):
- Default font: 19 / (3840/640) = **3.17** (extremely small!)
- Reference width of 640px is hardcoded from original game design
- **No DPI scaling support exists** in the codebase

### Window Size Reporting

```cpp
// CEngine::GetWindowSize() - colobot-base/src/graphics/engine/engine.cpp:628
glm::ivec2 CEngine::GetWindowSize()
{
    return m_size;  // Returns ACTUAL pixel dimensions, not scaled
}
```

The engine reports actual pixel dimensions, not DPI-scaled dimensions.

## Code Locations

### Font Size Calculation
- **File**: `colobot-base/src/ui/displayinfo.cpp`
- **Line**: 899
- **Function**: `CDisplayInfo::ViewDisplayInfo()`

### Font Rendering
- **File**: `colobot-base/src/graphics/engine/text.cpp`
- **Function**: `CText::GetFontPointSize()` (line 1244)

### Settings
- **File**: `colobot-base/src/common/settings.h`
- **Default**: `float m_fontSize = 19.0f`
- **Range**: 9.0f to 24.0f

## What's Missing

The codebase has **NO HiDPI/DPI scaling support**:

- ❌ No `SDL_GetDisplayDPI()` usage
- ❌ No `SDL_GetDisplayScale()` usage  
- ❌ No `SDL_GetWindowDisplayScale()` usage
- ❌ No content scaling or device pixel ratio
- ❌ No OS-level DPI detection

## Workarounds (Current)

### 1. Lower Resolution
Run game at lower resolution (e.g., 1920x1080):
```bash
SDL_VIDEO_FULLSCREEN_DESKTOP=0 LANGUAGE=es ./colobot
```

### 2. Maximum Font Size
Set font to maximum before starting:
```bash
# Set FontSize=24.0 in config
sed -i 's/FontSize=.*/FontSize=24.0/' ~/.config/colobot/colobot.ini
```

### 3. Disable Desktop Scaling
```bash
QT_AUTO_SCREEN_SCALE_FACTOR=0 GDK_SCALE=1 LANGUAGE=es ./colobot
```

## Proposed Solution

### Option 1: Detect Display DPI (Recommended)

Modify `CEngine::GetWindowSize()` or font calculation to account for DPI:

```cpp
// Add to CEngine or CApplication
float GetDPIScale()
{
    float dpi;
    if (SDL_GetDisplayDPI(displayIndex, &dpi, nullptr, nullptr) == 0)
    {
        // Standard DPI is 96
        return dpi / 96.0f;
    }
    return 1.0f;
}

// Then in ViewDisplayInfo():
float scale = GetDPIScale();
edit->SetFontSize(CSettings::GetInstancePointer()->GetFontSize()/(dim.x / (640.0f * scale)));
```

### Option 2: Use SDL's Display Scale (SDL 2.26+)

```cpp
float GetDisplayScale()
{
    return SDL_GetWindowDisplayScale(window);  // SDL 2.26+
}
```

### Option 3: Environment Variable Override

Add support for `COLOBOT_DPI_SCALE` environment variable:
```cpp
const char* scaleEnv = getenv("COLOBOT_DPI_SCALE");
float scale = scaleEnv ? std::stof(scaleEnv) : 1.0f;
```

## Files to Modify

1. **colobot-base/src/ui/displayinfo.cpp** (line 899)
   - `CDisplayInfo::ViewDisplayInfo()`

2. **colobot-base/src/ui/studio.cpp** (line 1117)
   - `CStudio::ViewEditScript()`

3. **colobot-base/src/graphics/engine/engine.h**
   - Add `GetDPIScale()` method

4. **colobot-base/src/graphics/engine/engine.cpp**
   - Implement DPI detection

## Testing

Test scenarios:
1. 1080p display (1920x1080) - should work normally
2. 1440p display (2560x1440) - should scale appropriately
3. 4K display (3840x2160) at 100% - text should be readable
4. 4K display at 200% OS scaling - text should match OS scale

## Related Issues

- Font size slider (EVENT_STUDIO_SIZE) uses same calculation
- May affect other UI elements that use `GetWindowSize()` for scaling
- Text rendering in `CText::GetFontPointSize()` compounds the issue

## References

- **Reference resolution**: 640x480 (original game design)
- **Font types**: `Gfx::FONT_SATCOM` for help text
- **Settings storage**: `colobot-base/src/common/settings.cpp`

## Priority

**Medium-High** - Affects usability on modern HiDPI displays (4K, Retina, etc.)

## Notes for Future Implementation

1. SDL 2.0.4+ has `SDL_GetDisplayDPI()`
2. SDL 2.26+ has `SDL_GetWindowDisplayScale()`
3. May need platform-specific code for Linux/macOS/Windows
4. Consider adding a "UI Scale" setting in graphics options
5. Test with fractional scaling (125%, 150%, 175%)

---

**Investigated**: 2026-02-14
**Status**: Documented, needs implementation
**Estimated Effort**: 2-4 hours for basic DPI support
