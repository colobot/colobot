# Compile and install instructions

## Source and data files

Colobot source files can be downloaded from Github repository (https://github.com/colobot/colobot). You can either download
the repository as a ZIP archive, or, clone the repository using git or a GUI frontent for git.

Make sure that once you download/clone the repository, you have the neeeded data files in `data/` subdirectory.These files
are provided as git submodule, hosted at a separate Github repository (https://github.com/colobot/colobot-data).
If you don't have them, you can either download the repository manually and unpack its content into `data/` or,
if you're working with git cloned repository, `git submodule update --init` will download the data submodule repository.


## Compiling on Windows

#### Compiling with MSYS/MinGW

If you like challenges ;-), you can try to compile Colobot directly under MSYS/MinGW (http://www.mingw.org/wiki/MSYS).
You need to manually compile about 20 packages and resolve many problems. Fortunately, the developers took pity on you,
and provide a download package containing all the necessary libraries and tools.

To use this package, you must first install a vanilla MSYS/MinGW enviromnent. To do this, download and run
mingw-get installer (http://sourceforge.net/projects/mingw/files/Installer/mingw-get-inst/).
When installing, select **all** possible packages in the installer.

Next, download the development package available at Colobot site (http://colobot.info/) and unpack the files
from the archive to MinGW directory. This should provide a working environment, including CMake and
all necessary packages. However, make sure you get the right package. There are slight changes between GCC 4.6 and 4.7,
especially with boost library which will result in build failure or error in runtime.

To compile Colobot, `cd` to directory with sources and run:
 $ cmake -G "MSYS Makefiles" -DCMAKE_BUILD_TYPE=Release .
and then:
 $ make

Everything should compile just fine. If you see any errors, it most likely means missing libraries or invalid installation.
Warnings may occur, but are mostly harmless.

You'll get the binary `colobot.exe`, which you can run directly, pointing it to the data directory:
 $ colobot.exe -datadir ./data

You can also install Colobot in your system using  
 $ make install

The default install path is `C:\Program Files\colobot`, but you can change it by adding `-DCMAKE_INSTALL_PREFIX="C:\your\path"`
to CMake arguments.

See also "Hints and notes" below on some useful advice.

#### Cross-compiling using MXE

MXE (http://mxe.cc/) is a very good cross-compiling framework, complete with a suite of libraries
that make it extremely easy to port applications to Win32. It runs on pretty much any *nix flavor and generates generic,
statically linked Win32 binaries. More information is available in INSTALL-MXE.md file.


## Compiling on Linux

Depending on your distribution, you'll need to install different packages, so here's just an outline, the details will
be different for different distros:
 * recent compiler (GCC >= 4.6 or a newer clang) since we are using some features of C++11.
 * CMake >= 2.8.
 * Boost >= 1.51 (header files + components: filesystem and regex)
 * SDL >= 1.2.10
 * SDL_image >= 1.2
 * SDL_ttf >= 2.0
 * GLEW >= 1.8.0
 * libpng >= 1.2
 * gettext >= 0.18
 * libsndfile >= 1.0.25
 * libvorbis >= 1.3.2
 * libogg >= 1.3.0
 * OpenAL (OpenAL-Soft) >= 1.13

Instructions for compiling are universal:
  $ cmake -DCMAKE_BUILD_TYPE=Release .
  $ make

Everything should compile just fine. If you see any errors, it most likely means missing libraries. Warnings may occur,
but are mostly harmless.

You'll get the binary in `bin/colobot`, which you can run directly, pointing it to the data directory:
 $ bin/colobot -datadir ./data

To install colobot in the system, you can run:
 $ make install

The default installation path is `/usr/local/` but you can change it by adding `-DCMAKE_INSTALL_PREFIX="/your/custom/path"`
to CMake arguments.

See also "Hints and notes" below on some useful advice.


## Compiling on other platforms

We haven't checked other platforms yet but the code isn't particularly tied to any compiler or platform, so in theory
it should work. If you can, please try to compile the code on your platform and let us know how it goes.


## Hints and notes

CMake has a very useful feature - out-of-source builds - using a separate directory for the output of CMake and compiler.
This way, you can keep clean the directory with your source files. Example of use (starting from directory with sources):
 $ mkdir build/
 $ cd build/
 $ cmake ../
 $ make


If you want to submit debug reports, please use special Debug and Dev builds (`-DDEV_BUILD=1 -DCMAKE_BUILD_TYPE=Debug`)
and run the game in debug mode and with logging on higher level (commandline arguments: `-loglevel debug`).
Also, `-help` will give full list of available arguments.


If you encounter any problems, you can get help at our forum or IRC channels.
