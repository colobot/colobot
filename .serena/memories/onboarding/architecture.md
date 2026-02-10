# Colobot Architecture

## High-level structure

Colobot is a modular C++20 game engine and educational learning environment. The project is split into:

1. **colobot-common/**: Low-level shared utilities (data structures, math, common helpers).

2. **CBot/**: Scripting language parser and runtime. Domain-specific code; **no exceptions** allowed in this module.

3. **colobot-base/**: Main game engine:
   - `app/`: Application/game loop, initialization
   - `common/`: Common game utilities (logging, macros, resource text TR() macros)
   - `graphics/`: Rendering and graphics pipeline (OpenGL, models, particles)
   - `level/`: Level/mission management
   - `math/`: Math utilities (vectors, matrices, geometry)
   - `object/`: Game objects (robots, buildings, resources)
   - `physics/`: Physics simulation
   - `script/`: Script integration (loading CBot scripts and running them)
   - `sound/`: Audio (OpenAL, sound effects, music)
   - `ui/`: User interface (UI controls, HUD, menus)

4. **colobot-app/**: Main executable entry point (minimal, mostly calls colobot-base).

5. **CBot-specific**: Domain-specific language for in-game programming; students write CBot code to control game characters.

6. **Test framework**: GoogleTest (`lib/googletest`), tests in `test/` directory. **Note**: Tests are optional and disabled by default. Enable with `-DTESTS=ON`.

## Translation system

The project uses **gettext** for i18n:

- **`po/CMakeLists.txt`** defines the translation build process.
- **POT file** (`colobot.pot`): Template containing all translatable strings.
- **PO files** (`*.po`): Language-specific translation files (e.g., `fr.po`, `de.po`, `es.po` if it existed).
- **Extraction**: `xgettext` extracts strings from source files (`app.cpp`, `restext.cpp`, `script.cpp`).
- **Compilation**: `msgmerge` merges new strings; `msgfmt` compiles PO → binary MO files for distribution.

## Key conventions

- **Translatable strings**: Use `TR("string")` macro in code (defined in `colobot-base/src/common/restext.cpp`).
- **String resources**: Some strings from `restext.cpp` are marked for translation.
- **Installation**: Compiled `.mo` files go to `${COLOBOT_INSTALL_I18N_DIR}/{lang}/LC_MESSAGES/`.
