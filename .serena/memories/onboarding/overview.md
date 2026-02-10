Colobot is a C++20 open-source educational game and learning environment. The repository contains several components:

- `colobot-common/`: shared utilities and low-level helpers.
- `CBot/`: in-game scripting language parser/runtime (CBot code avoids exceptions).
- `colobot-base/`: game engine, physics, objects, graphics.
- `colobot-app/`: the `colobot` executable (entrypoint).
- `data/`: external submodule containing game data (required for running the game).

Key build notes:
- CMake-based build (CMake Presets provided in `CMakePresets.json`).
- Recommended build flow (Linux):
  cmake --preset Linux-CI-gcc
  cmake --build --preset Linux-CI-gcc -- -j$(nproc)
  cmake --build --preset Linux-CI-gcc --target install

Testing:
- Unit tests use GoogleTest and are run with ctest in `build/` or by running `./Colobot-UnitTests` directly.

Coding conventions:
- 4-space indentation, braces on new lines, function names start with uppercase, classes start with `C`.
- Public APIs documented with Doxygen.
- Avoid exceptions in `CBot/` code.

Developer utilities:
- `vcpkg` is optionally used for dependencies; `vcpkg` lives often in `/usr/local/vcpkg` when bootstrapped.
- Linter: project uses `colobot-lint` and lint checks are available via `scripts/git-diff-check-colobot-lint.sh`.

Further reading: `INSTALL.md`, `CONTRIBUTING.md`, and `CMakePresets.json`.
