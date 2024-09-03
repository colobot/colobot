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

You can also compile directly on Windows with MSYS2/MinGW-w64 or Visual Studio since version 2015.

#### Compiling with MSYS2/MinGW-w64

See the [INSTALL-MSYS2.md](docs/INSTALL-MSYS2.md) file for details.

#### Compiling with MSVC (Visual Studio)

See [this wiki page](https://github.com/colobot/colobot/wiki/How-to-Build-Colobot:-Gold-Edition-Using-MSVC) for details.


### Compiling on Linux

Since there are so many Linux flavors, it is difficult to write generic instructions. However, here is the general gist of what
you will need to compile colobot.

You will need:
 * C++20 compiler
 * CMake >= 3.21
 * SDL2
 * SDL2_image
 * SDL2_ttf
 * GLEW
 * libpng
 * gettext
 * libsndfile
 * libvorbis
 * libogg
 * OpenAL (OpenAL-Soft)
 * PhysFS
 * glm
 * wget (to download music files)
 * oggenc (to transcode music files)

On Ubuntu (and probably any other Debian-based system), you can use the following command to install all required packages:
```
  $ apt-get install build-essential cmake libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev libsndfile1-dev libvorbis-dev libogg-dev libpng-dev libglew-dev libopenal-dev libphysfs-dev gettext git po4a vorbis-tools libglm-dev
```

Ubuntu 20.04 users have to also get gcc-11 and cmake >= 3.21, which might not be available in default repositories. To install them you need to:
1. Add ubuntu-toolchain PPA to your system `sudo add-apt-repository ppa:ubuntu-toolchain-r/test`
2. Install gcc-11 and set it as default compiler `sudo apt install gcc-11 g++-11 && sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-11 100 --slave /usr/bin/g++ g++ /usr/bin/g++-11 --slave /usr/bin/gcov gcov /usr/bin/gcov-11 && sudo update-alternatives --set gcc /usr/bin/gcc-11`
3. Install cmake snap package `sudo snap install cmake`

Make sure you install the packages along with header files (often distributed in separate *-dev packages). If you miss any requirements,
CMake should warn you.

To compile colobot, run your favorite shell and download colobot source files:
```
  $ git clone https://github.com/colobot/colobot.git /path/to/colobot/sources
  $ cd /path/to/colobot/sources
  $ git submodule update --init
```
If you want to compile development branch, change the first command to:
```
  $ git clone -b dev https://github.com/colobot/colobot.git /path/to/colobot/sources
```
If you also want to download optional music files, add --recursive to third command like so:
```
  $ git submodule update --init --recursive
```
It is recommended that you create a build directory:
```
  $ mkdir build
  $ cd build
```
Now to configure CMake:
```
   $ cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/some/prefix ..
   ## Ubuntu 20.04 users with both apt and snap version of CMake needs to pass path to snap version, like this:
   $ /snap/cmake/current/bin/cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/some/prefix ..
```
where `/some/prefix` is installation prefix where you want to put the game files. It could be a proper installation directory
if you want to install colobot in the system or simply temporary directory like `/tmp/colobot-temporary-install` if you just want to try it.
You can also use Clang as the compiler. In that case, before issuing cmake, set the following variables:
```
  $ export CC=clang CXX=clang++
```
Then to compile:
```
  $ make
```
Everything should compile just fine. If you see any errors, it most likely means missing libraries. Warnings may occur,
but are mostly harmless.

Now you need to install the game files:
```
  $ make install
```
You can now run the game from the prefix you specified. Note that colobot binary is installed in `games/` subdirectory.
So if you provided prefix "/some/prefix", you can run:
```
  $ /some/prefix/games/colobot
```

### Compiling on MacOS X

As of 0.1.2-alpha, we have added MacOS X support. See [INSTALL-MacOSX.md](docs/INSTALL-MacOSX.md)
file for details.


### Other platforms

The code isn't particularly tied to any compiler or platform, so in theory it should work on any platform provided you have
the required libraries there.  Also, other compilers than currently supported (GCC >= 4.7, Clang >= 3.1 and MSVC 2013) may happen to work with our code.
If you can, please try to compile the code on your platform and let us know how it goes.


## Error reports and debugging

Before reporting errors, please make sure you compile the dev branch to make sure that the issue is not yet fixed there. Also, search the
existing issues (https://github.com/colobot/colobot/issues) to check if a similar error has not been already reported.

If you want to submit error reports, please use special Debug and Dev builds (add `-DCMAKE_BUILD_TYPE=Debug` to CMake arguments)
and run the game in debug mode and with logging on higher level (commandline arguments: `-loglevel debug`).


## Language support

In order to run colobot in different language, you need to run with fully installed data files.
If you run a system with locale settings set to your language, colobot should auto-detect it and use appropriate language. If that does not happen,
you can force a specific language using option `-language xx` where xx is language code like `en` or `de`. `-help` will show all possible settings.


## Help

If you encounter any problems, you can get help at our forum or IRC channels.
