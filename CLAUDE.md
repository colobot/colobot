# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build

```sh
git clone --recursive https://github.com/colobot/colobot.git
cd colobot && mkdir build && cd build
cmake -DCOLOBOT_DEVELOPMENT_MODE=ON -DTESTS=ON ..
make
./colobot -datadir data/
```

CMake presets are available for CI environments: `Linux-CI-gcc`, `Linux-CI-clang`, `MacOS-CI`, `Windows-CI`.

```sh
cmake --preset Linux-CI-gcc
cmake --build --preset Linux-CI-gcc
```

Data files live in the `data/` git submodule — initialize with `git submodule update --init`.

## macOS build (Homebrew)

On macOS, install the **official** Homebrew (not Apple-internal) to `~/homebrew`:

```sh
mkdir -p ~/homebrew && curl -L https://github.com/Homebrew/brew/tarball/master | tar xz --strip-components 1 -C ~/homebrew
HOMEBREW_NO_VERIFY_ATTESTATIONS=1 ~/homebrew/bin/brew install cmake sdl2 sdl2_image sdl2_ttf glew physfs libsndfile libvorbis glm
```

Then configure pointing at it and forcing OpenGL core profile (required on macOS):

```sh
mkdir build-dev && cd build-dev
cmake -DCMAKE_BUILD_TYPE=Debug -DCOLOBOT_DEVELOPMENT_MODE=ON -DTESTS=ON \
  -DCMAKE_PREFIX_PATH="$HOME/homebrew" ..
make -j$(sysctl -n hw.logicalcpu)
```

Run (development mode copies `data/` into the build dir, so `-datadir` must point there):

```sh
./colobot -datadir data/ -glversion 3.3 -glprofile core -resolution 1600x1200
```

## Visual verification (macOS)

`scripts/verify-visual.sh` launches the game, waits for the window, captures a screenshot, and quits:

```sh
bash scripts/verify-visual.sh build-dev/colobot 1600x1200 /tmp/verify.png
```

Claude can then `Read /tmp/verify.png` to inspect the rendered output visually.
Use `COLOBOT_DATADIR=/path/to/data` to override the auto-detected data directory.

## Tests

```sh
cd build
./Colobot-UnitTests        # or: make test / ctest -V .
```

Tests are under `test/unit/`. Enable with `-DTESTS=ON` at configure time.

## Linting

Coding style is enforced by [colobot-lint](https://github.com/colobot/colobot-lint) (separate repo). Enable integration with `-DCOLOBOT_LINT_BUILD=ON`. PRs that break style must be fixed before merge.

## Architecture

The engine is split into several static libraries compiled together:

- **`colobot-app/`** — `main()` entry point; bootstraps `CApplication`
- **`colobot-base/src/`** — core engine; the majority of code lives here
  - `app/` — `CApplication`: SDL window, event loop, input (`CInput`, `CController`)
  - `graphics/engine/` — `CEngine`: 3D scene rendering; `CText`: SDL_ttf-based text rendering with per-glyph OpenGL texture atlas
  - `graphics/core/` — abstract device interface; `graphics/opengl33/` — OpenGL 3.3 implementation
  - `level/robotmain.cpp` — `CRobotMain`: game coordinator, level loading/state machine
  - `object/` — `CObject` base class + all robots, buildings, plants, aliens; `task/` for robot task system
  - `physics/` — collision and gravity simulation
  - `script/` — bridges `CBot` language to game callbacks (`scriptfunc.cpp`)
  - `ui/` — all 2D interface panels, controls, screen managers
- **`CBot/`** — the in-game C-like scripting language interpreter (used by players to program robots)
- **`colobot-common/`** — shared string utilities and types

**Data flow:** `CApplication` event loop → `CRobotMain::Update()` → physics update → `CEngine::Render()`. Robot scripts run via `CScript` → `CBot` interpreter → callbacks in `scriptfunc.cpp`.

**Coordinate systems:** Interface positions are in `[0,1]` normalized space; conversion to/from window pixels goes through `CEngine::InterfaceToWindowCoords()` / `WindowToInterfaceSize()`.

**Text rendering (`graphics/engine/text.cpp`):**
- `CText::GetFontPointSize(float size)` converts logical font size to pixel point size, scaling by `windowSize / REFERENCE_SIZE(800,600)` for HiDPI support.
- Glyphs are cached per `(FontType, pointSize)` in `CachedFont::cache` (keyed by `CodePoint`).
- Atlas textures (`FontTexture`) are allocated in `m_fontTextures`; tile size is the next power-of-two of each glyph's pixel dimensions. If a glyph exceeds the current atlas size, `ResizeFontTexture()` grows it and reloads all fonts.

## Coding style

- 4-space indentation, Unix line endings, no trailing whitespace
- Braces on their own lines (Allman style)
- Classes: `CClassName`; structs/enums: `SomeName`; enum values: `ALL_CAPS_WITH_PREFIX`
- Functions: `UpperCamelCase()`; accessors: `GetValue()` / `SetValue()`
- C++ casts only (`static_cast`, etc.); no C-style casts
- Include order in `.cpp`: own header first, then local headers alphabetically, then system/third-party headers
- Full namespace qualifiers preferred (e.g. `Math::Foo()` not `using namespace Math`)
- PRs go to the `dev` branch, never `master`

## Contributing workflow

1. Discuss the feature/bug in the GitHub issue and wait for the `accepted` label before writing code.
2. Check that nobody else is already assigned to the issue.
3. Commits must not break the build or tests; keep them focused and descriptive.
4. All PRs target `dev`. The maintainers merge to `master` on releases.
