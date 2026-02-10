Build and test
- Configure and build (out-of-source):
  mkdir -p build && cd build && cmake .. -G Ninja
  cmake --build . -- -j$(nproc)
- Run unit tests:
  cd build && ctest --output-on-failure
- Run test binary directly:
  ./Colobot-UnitTests

Development
- Update submodules:
  git submodule update --init --recursive
- Linting and formatting:
  scripts/git-diff-check-colobot-lint.sh
  # Project uses custom colobot-lint; follow CONTRIBUTING.md

Running
- Run the built executable:
  build/colobot-app/colobot (or installed prefix games/colobot)

Utilities
- Install vcpkg dependencies (optional):
  /usr/local/vcpkg/vcpkg install <packages>

Notes
- Project uses CMake presets in CMakePresets.json for CI parity
- Data submodule (`data/`) is required for the game to run; see INSTALL.md
