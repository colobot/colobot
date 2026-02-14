# Colobot — AI agent quickstart & coding conventions 🔧

Purpose: give a focused, actionable summary so an AI coding agent can be productive immediately in this repository.

## Big picture (what to know first) 💡
- This is a C++20 game project split into clear components:
  - `colobot-common/` — shared utilities and low-level helpers
  - `CBot/` — the in-game scripting language parser/runtime (domain-specific code; **no exceptions** in CBot code)
  - `colobot-base/` — game engine, physics, objects, graphics
  - `colobot-app/` — the `colobot` executable (entrypoint)
  - `tools/`, `desktop/`, `po/` — tooling, platform packaging, and translations
- Game data lives in a separate submodule (`data/` from `colobot-data`) and **must** be present/installed (see `INSTALL.md`).

## How to build & run (exact commands) ▶️
- Clone with submodules (dev branch recommended):
  - `git clone -b dev --recurse-submodules https://github.com/colobot/colobot.git`
- Typical Linux quickstart (use CMake presets for CI parity):
  - `cmake --preset Linux-CI-gcc` (or `Linux-CI-clang` / `MacOS-CI` / `Windows-CI`)
  - `cmake --build --preset Linux-CI-gcc -- -j$(nproc)`
  - `cmake --build --preset Linux-CI-gcc --target install`
  - Run installed binary: `<install-prefix>/games/colobot` or run the built executable in `build/`.
- Running tests:
  - `cd build && ctest --output-on-failure` or run `./Colobot-UnitTests` (CI runs `./Colobot-UnitTests --gtest_output=xml:...`).
- Debugging & logs:
  - Configure with `-DCMAKE_BUILD_TYPE=Debug` and run the binary with `-loglevel debug`.
- Docs: build with `make doc` (or `cmake --build . --target doc`) — see CI `doc` job.

## Project-specific flags and tooling 🔧
- Useful CMake options: `TESTS`, `TOOLS`, `DESKTOP`, `COLOBOT_LINT_BUILD`, `CBOT_STATIC`, `PORTABLE`, `PORTABLE_SAVES` (also see `CMakePresets.json`).
- Linter: repo uses a separate `colobot-lint` tool. See `cmake/colobot-lint.cmake` and `scripts/git-diff-check-colobot-lint.sh`.
- Dependencies: system packages or `vcpkg` (`vcpkg.json`). CI uses `cmake --preset` to standardize environments.

## Tests & CI notes ✅
- Unit tests use GoogleTest and `gtest_discover_tests` in `test/CMakeLists.txt`.
- CI builds with strict flags (e.g., `-Werror` in `Linux-CI-gcc`) — prefer using CMake presets to reproduce CI behavior.
- CI runs tests by executing the unit test binary and uploads results (`.github/workflows/build.yml`).

## Coding conventions & review checklist 🧾
- Follow `CONTRIBUTING.md` conventions (important highlights):
  - Indent with **4 spaces**, braces on **new lines**; function names start with uppercase; classes start with `C`.
  - Includes order: local header, module headers, then system headers.
  - Document public APIs with Doxygen comments.
  - Prefer tests in `test/` near the code you change.
  - Avoid exceptions in `CBot/` code (no automatic memory management there).
- When making changes:
  - Keep commits small and focused; ensure build and tests pass locally.
  - Run linter and unit tests before opening PR.

## Where to look first (short list) 🔎
- `INSTALL.md` — build and install notes
- `CMakePresets.json` — canonical build presets used by CI
- `CONTRIBUTING.md` — coding style & PR rules
- `CMakeLists.txt` (top-level) — project structure & important options
- `colobot-common/`, `CBot/`, `colobot-base/`, `colobot-app/` — primary code areas
- `test/` and `lib/googletest` — unit test setup
- `.github/workflows/build.yml` — exact CI steps and expectations

> Note: be conservative with changes touching `data/` (submodule), packaging or CI presets — coordinate on issues/Discord if unsure.

## Translation System Quick Reference 📝

### Language Codes
- Spanish uses `Title.S` not `Title.E` (collision with English)
- English: `help.E.txt`, Spanish: `help.S.txt`

### Terminology Rules
- **Use "bot"** (never "robot") - game lore (COLO-BOT)
- **Energy cells**: "celdas de energía" (NOT "células de energía")
- **Code keywords**: Keep English (`turn()`, `fire()`, `motor()`, `while`, `if`, `radar`)
- **Preserved tags**: `<c/>`, `<n/>`, `<button 22/>`, `<code>...</code>`

### Key Translation Files
- Main UI: `po/es.po` (659 strings)
- Help: `data/help/*/po/es.po`
- Levels: `data/levels/*/chapter00X/level00Y/po/es.po`

### Build Translations
```bash
cd build && make -j4
cmake --install . --prefix /tmp/colobot-install
LANGUAGE=es /tmp/colobot-install/games/colobot
```

### Validate
```bash
msgfmt -c po/es.po  # Check syntax
grep -c '^msgstr ""$' po/es.po  # Check empty translations
```

### Common Fixes
```bash
# Energy cells terminology
find . -name "es.po" -exec sed -i 's/células de energía/celdas de energía/g' {}

# Robots → bots in factory context
sed -i 's/<a object|factory>robots/<a object|factory>bots/g' data/help/object/po/es.po
```

### Full Guide
See `docs/TRANSLATIONS.md` for comprehensive documentation.

Follow these steps for each interaction:

1. User Identification:
   - You should assume that you are interacting with default_user
   - If you have not identified default_user, proactively try to do so.

2. Memory Retrieval:
   - Always begin your chat by saying only "Remembering..." and retrieve all relevant information from your knowledge graph
   - Always refer to your knowledge graph as your "memory"

3. Memory
   - While conversing with the user, be attentive to any new information that falls into these categories:
     a) Basic Identity (age, gender, location, job title, education level, etc.)
     b) Behaviors (interests, habits, etc.)
     c) Preferences (communication style, preferred language, etc.)
     d) Goals (goals, targets, aspirations, etc.)
     e) Relationships (personal and professional relationships up to 3 degrees of separation)

4. Memory Update:
   - If any new information was gathered during the interaction, update your memory as follows:
     a) Create entities for recurring organizations, people, and significant events
     b) Connect them to the current entities using relations
     c) Store facts about them as observations
