# README for developers

This file contains a ton of information useful for development of the game

## Repository setup

All the repositories related to Colobot can be found on our GitHub organization page: https://github.com/colobot

### Main code repository

This is the repository you are currently in.

This repository contains all the source files of Colobot, along with some 3rd party libraries, testing framework and build files for CMake.

### Data repository

The data repository is available at: https://github.com/colobot/colobot-data

This repository contains the data files necessary to run the game. These are level files, textures, models, sounds, music, help files, translations, etc. It contains many binary files, and so, it may grow up to considerable size. If that becomes a problem, we may remove some old revisions of binary files to free up space.

### Branch setup

Current setup is as follows:

* branch **master** - will always contain the best-playable version of the project so that new users could download and compile the project without problems. This branch is not intended for actual development but an integration branch for more "public" releases. The changes should be pulled from general development branch - *dev*. This branch will also have tags at certain commits to denote releases.
* branch **dev** - development branch. This branch is used for general development. Changes committed here should be either pull requests implementing whole features, or incremental commits that do not change many files.

Other **dev-*** branches may be created as needed, for work on major rewriting, or focusing on a set of features.

## 3rd party libraries

3rd party libraries are bundled in `lib/`. They are provided for ease of use since the standard packages are rare in OS distributions.

In case of GTest and GMock, CMake tries to detect if they are available in the system. If so, the system-provided versions will be used.

## CMake build system

The build system is as follows:

* `CMakeLists.txt` - definition of project, list of required packages, build type setup, general compiler options and reference to src subdirectory,
* `src/CMakeLists.txt` - defines the main colobot target,
* `src/CBot/CMakeLists.txt` - defines the CBot library target,
* `src/tools/CMakeLists.txt` - defines tool program targets,
* `data/CMakeLists.txt` - separate file in data submodule, includes routines for creating translations, manpage, icons, etc. and installing them with program.

Test build system is discussed below.

CMake sets some `#defines` which are passed to code in generated headers `common/config.h` and `common/version.h`.

There are only a few include directories specified:

* the main source directory `src/`,
* CMake binary directory (because of generated `common/config.h`),
* `lib/` directory with 3rd party libraries.

Because of the above, include paths in source should always feature the full path from src, e.g. `#include "object/auto/auto.h"`, even if the file lies in the same directory. This will make it easier to move files around and change the `#include` lines with automated replace scripts (some of which are available in *tools/*).

Note that the recommended way of building the project is to use separate build directory, where CMake will generate all targets. In this way, you can keep a clean source directory. The following shell commands illustrate this usage:

```sh
  git clone --recursive https://github.com/colobot/colobot.git
  cd colobot
  mkdir build
  cd build
  cmake -D COLOBOT_DEVELOPMENT_MODE=ON ..
  make
  ./colobot -datadir data/
```

## Tests organization

Tests are kept in `test/` directory which includes:

* `test/cbot` - CBOT interpreter for test purposes,
* `test/unit` - automated unit tests.

Each test directory contains own `CMakeLists.txt` specifying targets. Note however that the only targets added as automated tests in CMake are in `test/unit` directory. The other targets are used as development support tools, not automated tests.

Tests can be enabled or disabled using CMake option TESTS (OFF by default). To run the automated tests (you must be in the build directory):

```
  ./Colobot-UnitTests
  # or:
  make test
  # or:
  ctest -V .
```

We use [Google Test](https://github.com/google/googletest) for unit testing and [Hippomocks](https://github.com/dascandy/hippomocks) for mocking objects and function calls.

Note that currently there are very few well-tested parts of code. It is quite impossible to write unit tests for all the code we have but we should test as much code as possible, if not at unit level, then at module level. Hence the test environments and code snippets that enable us to test parts of code without actually running the full-fledged game.

## CI and automated builds

Currently we use [GitHub Actions](https://github.com/colobot/colobot/actions) to run tests and provide compiled game binaries.

Testers are encouraged to use these packages to test the game and report any problems.

## Code documentation

[Doxygen](https://www.doxygen.nl/index.html) is used as the documentation tool.

The documentation is extracted from comment blocks in code by running `make doc`. The resulting HTML files are available in ''doc/'' inside the build directory. Generated documentation files '''will not''' be included in the repository. The idea is that anyone can generate the documentation at any time, in his local copy. GitHub Actions also builds documentation after every commit, see https://github.com/colobot/colobot/actions

Diagram generation (class inheritance, include dependencies, etc.) is disabled for now to speed up the generation process but you can of course enable it in your local copy.

Currently, only a few classes and structs are documented properly. If you can, please expand the documentation wherever possible.

## Coding rules

Please refer to the [CONTRIBUTING.md](CONTRIBUTING.md#coding-style) file.
