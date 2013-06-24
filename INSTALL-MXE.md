# Cross-compiling with MXE

MXE works for any BSD-compatible system (including Linux).
It is a complete package with cross-compiler to Win32 (a MinGW variant)
and includes scripts to automatically download and build many 3rd party
libraries and tools.

To cross-compile Colobot using MXE:

1. See the MXE website (http://mxe.cc) for list of required packages and make sure
   you have them installed.

2. Download MXE and unpack it in the directory, where you want to keep it
   permanently. During the build, MXE will write that path to many files,
   so moving that directory can be tricky.

3. `cd` to the MXE root directory.
   It already contains a universal Makefile for everything.
   Usage is simply `make name_of_package`.
   It will automatically check for dependencies, etc.
   The packages will be installed in the MXE directory under `usr/`.

   You need to `make gcc` first for basic compiler and then do the same
   for some additional libraries. In the end, you should have the following
   packages installed (this is the final listing of `usr/installed/`):
    * binutils
    * boost
    * bzip2
    * check-requirements
    * expat
    * flac
    * flac
    * freetype
    * gcc
    * gcc-gmp
    * gcc-mpc
    * gcc-mpfr
    * gettext
    * glew
    * jpeg
    * libiconv
    * libpng
    * libsndfile
    * libtool
    * mingwrt
    * ogg
    * openal
    * portaudio
    * sdl
    * sdl_image
    * sdl_ttf
    * tiff
    * vorbis
    * w32api
    * xz
    * zlib

4. Now `cd` to directory with colobot sources. To cross-compile a CMake project,
   you have to specify a CMake toolchain file. MXE has such file in MXE's directory:
   `usr/i686-pc-mingw32/share/cmake/mxe-conf.cmake`
   Toolchain file is specified thus:`cmake -DCMAKE_TOOLCHAIN_FILE=/path/to/mxe-conf.cmake .`
   CMake files in Colobot should detect that MXE is being used and they will
   modify flags, paths, etc. as required. You should not run into any problems.
   *Note:* you may also want to use a separate out-of-source build directory for MXE.

5. `make` should now compile the game with the resulting exe in `bin/colobot.exe`.
   The exe is linked against all libraries *statically*, so there are no dependencies
   on external DLLs. However, the resulting binary will be huge with all these libraries,
   so you might want to do: `strip bin/colobot.exe`.
