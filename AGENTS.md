# Colobot: Gold Edition - AI Agent Guide

This file provides essential information for AI coding agents working in the Colobot repository.

## Quick Start Commands

### Building the Project
```bash
# Configure with CMake preset (recommended for CI parity)
cmake --preset Linux-CI-gcc  # or Linux-CI-clang, MacOS-CI, Windows-CI

# Build the project
cmake --build --preset Linux-CI-gcc -- -j$(nproc)

# Install
cmake --build --preset Linux-CI-gcc --target install

# Run the game
./build/colobot  # or <install-prefix>/games/colobot
```

### Running Tests
```bash
# Run all tests
cd build && ctest --output-on-failure

# Run single test file (for specific testing)
./Colobot-UnitTests --gtest_filter="YourTestSuite.YourTestMethod"

# Run tests with XML output (CI format)
./Colobot-UnitTests --gtest_output=xml:test-results.xml
```

### Code Quality Checks
```bash
# Run linter (requires colobot-lint tool)
./scripts/git-diff-check-colobot-lint.sh

# Build with lint checking enabled
cmake --preset Linux-CI-gcc -DCOLOBOT_LINT_BUILD=ON
```

## Project Structure

### Core Components
- **colobot-common/**: Shared utilities and low-level helpers
- **CBot/**: In-game scripting language parser/runtime (NO exceptions allowed)
- **colobot-base/**: Game engine, physics, objects, graphics
- **colobot-app/**: Main colobot executable (entrypoint)
- **tools/**: Development and build tools
- **desktop/**: Platform-specific packaging
- **test/**: Unit tests using GoogleTest framework

### Key Files
- **CMakePresets.json**: Canonical build presets used by CI
- **vcpkg.json**: Dependencies specification
- **CMakeLists.txt**: Main project configuration
- **data/**: Game data (git submodule from colobot-data)

## Build Options

### Useful CMake Variables
- `TESTS=ON/OFF`: Enable/disable unit tests
- `TOOLS=ON/OFF`: Build development tools
- `DESKTOP=ON/OFF`: Build desktop integration
- `COLOBOT_LINT_BUILD=ON/OFF`: Enable lint checking during build
- `CBOT_STATIC=ON/OFF`: Static linking for CBot
- `PORTABLE=ON/OFF`: Portable build mode
- `PORTABLE_SAVES=ON/OFF`: Portable save file location

## Code Style Guidelines

### Formatting
- **Indentation**: 4 spaces (NO tabs)
- **Braces**: Opening brace on new line
- **Line endings**: Unix (LF)
- **Trailing whitespace**: Not allowed

### Naming Conventions
- **Functions**: PascalCase (e.g., `FooBar()`)
- **Classes**: Prefix with 'C' (e.g., `class CSomeClass`)
- **Accessors**: `SetValue()`, `GetValue()`
- **Structs/Enums**: PascalCase (e.g., `struct SomeStruct`)
- **Enum values**: PREFIX_ALL_CAPS (e.g., `SOME_ENUM_VALUE`)
- **Constants**: ALL_CAPS (e.g., `const int MAX_SPACE = 1000;`)

### C++ Style Rules
- Use C++20 standard
- Use C++-style casts: `static_cast<Type>(value)` instead of `(Type)value`
- No global variables - use static class members
- Prefer `const` over `#define`
- Use STL classes where appropriate
- Exceptions allowed except in CBot code (memory management concerns)
- Provide full namespace qualifiers: `Math::MultiplyMatrices`

### Include Order
1. Associated header (in .cpp files): `#include "app/app.h"`
2. Local includes (alphabetical): `#include "common/logger.h"`
3. System includes: `#include <vector>`, `#include <SDL/SDL.h>`

### Documentation
- Document public APIs with Doxygen comments
- Comments should explain **why**, not **what**
- Write unit tests for new code in `test/` subdirectories

## Development Workflow

### Making Changes
1. Ensure code follows style guidelines
2. Write/update unit tests
3. Run linter locally
4. Build and test with CI presets
5. Keep commits small and focused

### Testing Strategy
- Unit tests use GoogleTest framework
- Tests discovered automatically via `gtest_discover_tests`
- CI runs tests with `-Werror` (treat warnings as errors)
- Place tests near the code they test

### Memory Management
- RAII preferred throughout codebase
- **No exceptions in CBot code** (manual memory management)
- Use smart pointers and STL containers

## CI Configuration

### Presets by Platform
- **Linux-CI-gcc**: GCC-based Linux builds
- **Linux-CI-clang**: Clang-based Linux builds  
- **MacOS-CI**: macOS builds
- **Windows-CI**: MSVC-based Windows builds

### CI Requirements
- Builds with `-Werror` (warnings as errors)
- Runs full unit test suite
- Generates documentation with Doxygen
- Uploads test results as XML

## Important Notes

### Game Data Requirements
- Game data lives in separate submodule (`data/`)
- Submodule **must** be present for builds/tests
- Use `git submodule update --init --recursive` after cloning

### Dependencies
- System packages preferred, fallback to vcpkg
- See `INSTALL.md` for platform-specific requirements
- CI uses vcpkg for consistency

### Performance Considerations
- Debug builds: configure with `-DCMAKE_BUILD_TYPE=Debug`
- Use `-loglevel debug` for debugging output
- Profile builds use `-DCMAKE_BUILD_TYPE=RelWithDebInfo`

## Translation System

### Language Code Collision
Spanish uses `Title.S` not `Title.E` because English (`en`) already uses `E`:
- English: `help.E.txt`, `scene.E.txt`
- Spanish: `help.S.txt`, `scene.S.txt`
- Defined in `data/i18n-tools/scripts/common.py`

### Terminology Rules
- **Use "bot"** (never "robot") - part of game lore (COLO-BOT)
- **Energy cells**: "celdas de energía" (NOT "células de energía")
- **Code keywords**: Keep English (`turn()`, `fire()`, `motor()`, `while`, `if`, `radar`)
- **Tags preserved**: `<c/>`, `<n/>`, `<button 22/>`, `<code>...</code>`

### PO File Format
```po
#. type: Plain text
#: ../help/help.E.txt:2
#, no-wrap
msgid "Objective"
msgstr "Objetivo"
```

### Key Locations
- Main UI: `po/es.po` (659 strings)
- Help files: `data/help/*/po/es.po`
- Levels: `data/levels/*/chapter00X/level00Y/po/es.po`

### Build and Test
```bash
cd build && make -j4
cmake --install . --prefix /tmp/colobot-install
LANGUAGE=es /tmp/colobot-install/games/colobot
```

### Validation
```bash
msgfmt -c po/es.po                    # Validate syntax
grep -c '^msgstr ""$' po/es.po        # Check empty translations
```

### Common Fixes
```bash
# Fix energy cell terminology
find . -name "es.po" -exec sed -i 's/células de energía/celdas de energía/g' {}

# Fix robots → bots in factory context
grep -n '<a object|factory>robots' data/help/object/po/es.po
```

## Getting Help

- **Documentation**: `docs/` directory, `INSTALL.md`
- **Translation Guide**: `docs/TRANSLATIONS.md`
- **Issues**: GitHub issue tracker
- **Discussions**: GitHub Discussions
- **Real-time**: Discord server (linked in README)

When in doubt, prefer conservative changes and coordinate major modifications through issues or Discord discussions.