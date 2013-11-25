# Compile and install instructions

## Source and data files

Colobot source files can be downloaded from Github repository (https://github.com/colobot/colobot). You can either download
the repository as a ZIP archive, or, clone the repository using git or a GUI frontend for git.

Make sure that once you download/clone the repository, you have the needed data files in `data/` subdirectory.These files
are provided as git submodule, hosted at a separate Github repository (https://github.com/colobot/colobot-data).
If you don't have them, you can either download the repository manually and unpack its content into `data/` or,
if you're working with git cloned repository, `git submodule update --init` will download the data submodule repository.


## Important notes

It is highly recommended that while compiling, you do an out-of-source build, that is create a separate directory where all build files
will be created. This isolates the generated CMake files and makes it easy to clean them (simply remove the build directory)
as CMake lacks "make clean" command.

As of 0.1.2-alpha, running the game with source data directory is no longer supported as the data files
are now generated to support multiple languages. You have to perform installation, at least of the data files, to a destination
directory. If you fail to do that, and try to run the game with source data directory, the game will run, but you will not be able to access
any of the missions.


## Compilation

### Compiling on Windows

The recommended way of compiling for Windows is using Linux in a cross-compilation environment called MXE.
This is the way our build bot service (http://colobot.info/files/compiled.php) prepares the release packages.
You can also try to compile with MSYS/MinGW but this is more difficult.

#### Cross-compiling using MXE

MXE (M cross environment, http://mxe.cc/) is a very good cross-compiling framework, complete with a suite of libraries
that make it extremely easy to port applications to Win32. It runs on pretty much any *nix flavor and generates generic,
statically linked Win32 binaries. More information is available in
[INSTALL-MXE.md](https://github.com/colobot/colobot/blob/master/INSTALL-MXE.md) file.

#### Compiling with MSYS/MinGW

If you like challenges ;-), you can try to compile Colobot directly under MSYS/MinGW (http://www.mingw.org/wiki/MSYS).
You need to manually compile about 20 packages and resolve many problems. Fortunately, the developers took pity on you,
and provide a download package containing all the necessary libraries and tools.

To use this package, you must first install a vanilla MSYS/MinGW environment. To do this, download and run
mingw-get installer (http://sourceforge.net/projects/mingw/files/Installer/mingw-get-inst/).
When installing, select **all** possible packages in the installer.

Next, download the development package available at Colobot site (http://colobot.info/files/ - files named msys-devpack-*)
and unpack the files from the archive to MinGW directory. This should provide a working environment, including CMake and
all necessary packages. However, make sure you get the right package. There are slight changes between GCC 4.6 and 4.7,
especially with boost library which will result in build failure or error in runtime.

Once you have installed the development package, run the MSYS shell. This shell works as a hybrid *nix/Windows environment,
so you have regular bash commands but can specify paths using Windows syntax: "C:\some\path", CRLF is the endline separator and so forth.
CMake should automatically detect this build environment and use the Windows options to compile.

To compile colobot, change the directory to where you have downloaded the source files:
 $ cd "C:\path\to\colobot\sources"

It is recommended that you create a build directory:
 $ mkdir build
 $ cd build

Then you have to configure CMake. You should specify the following options:
 $ cmake -G "MSYS Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="C:\some\directory" ..
where "C:\some\directory" is the directory you want to run colobot from. It can be a proper installation path if you want
to install it in system, or some temporary directory like "..\colobot-temporary-install" if you just want to try the game.
You can also skip this argument and use the default install path: "C:\Program Files\colobot".
Make sure you specify "MSYS Makefiles" as the CMake generator because otherwise, the default for Windows is to use MSVC nmake
and it will not work.

Then to compile:
 $ make

Everything should compile just fine. If you see any errors, it most likely means missing libraries or invalid installation.
Warnings may occur, but are mostly harmless.

Now you need to perform the installation:
 $ make install

You should get all files ready to use under the installation prefix you specified. Run `colobot.exe` and enjoy the game.

### Compiling on Linux

Since there are so many Linux flavors, it is difficult to write generic instructions. However, here is the general gist of what
you will need to compile colobot.

You will need:
 * recent compiler (GCC >= 4.6 or a newer clang) since we are using some features of C++11
 * CMake >= 2.8
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
 * po4a >= 0.45 (to generate translated data files)

Make sure you install the packages along with header files (often distributed in separate *-dev packages). If you miss any requirements,
CMake should warn you.

To compile colobot, run your favorite shell and change the directory to where you downloaded colobot source files:
 $ cd /path/to/colobot/sources

It is recommended that you create a build directory:
 $ mkdir build
 $ cd build

Now to configure CMake:
  $ cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/some/prefix ..
where "/some/prefix" is installation prefix where you want to put the game files. It could be a proper installation directory
if you want to install colobot in the system or simply temporary directory like "/tmp/colobot-temporary-install" if you just want to try it.
You can also use clang as the compiler. In that case, before issuing cmake, set the following variables:
 $ export CC=clang CXX=clang++

Then to compile:
 $ make

Everything should compile just fine. If you see any errors, it most likely means missing libraries. Warnings may occur,
but are mostly harmless.

Now you need to install the game files:
 $ make install

You can now run the game from the prefix you specified. Note that colobot binary is installed in `games/` subdirectory.
So if you provided prefix "/some/prefix", you can run:
 $ /some/prefix/games/colobot

### Compiling on MacOS X

As of 0.1.2-alpha, we have added MacOS X support. See [INSTALL-MacOSX.md](https://github.com/colobot/colobot/blob/master/INSTALL-MacOSX.md)
file for details.

## Other platforms

The code isn't particularly tied to any compiler or platform, so in theory it should work on any platform provided you have
the required libraries there.  Also, other compilers than currently supported GCC >= 4.6 and Clang may happen to work with our code.
If you can, please try to compile the code on your platform and let us know how it goes.


## Error reports and debugging

Before reporting errors, please make sure you compile the dev branch to make sure that the issue is not yet fixed there. Also, search the
existing issues (https://github.com/colobot/colobot/issues) to check if a similar error has not been already reported.

If you want to submit error reports, please use special Debug and Dev builds (add `-DDEV_BUILD=1 -DCMAKE_BUILD_TYPE=Debug` to CMake arguments)
and run the game in debug mode and with logging on higher level (commandline arguments: `-loglevel debug`).


## Language support

In order to run colobot in different language, you need to run with fully installed data files.
If you run a system with locale settings set to your language, colobot should auto-detect it and use appropriate language. If that does not happen,
you can force a specific language using option `-language xx` where xx is language code like `en` or `de`. `-help` will show all possible settings.


## Help

If you encounter any problems, you can get help at our forum or IRC channels.
