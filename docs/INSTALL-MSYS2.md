This guide is written to help you with a process of compilation (the newest version of) Colobot: Gold Edition in a Windows environment. [MSYS2](http://sourceforge.net/projects/msys2/) will be used.

Why MSYS2?
----------

You might ask, why you would use this method instead of the other one? Firstly, let's answer a question "why not just use MSYS?".

The biggest problem with the compilation is **3rd party dependencies**. Installing compiler and tools like Git or Cmake is really nothing compared to trying to install endless number of libraries. Why is it that hard, you ask? Well, we, as the developers, would willingly help with this process for example by providing a package with all the needed stuff. Actually, there was a package once, but IT develops fast and new versions of software are released, including not only compiler, but also the libraries. This means that our package might work for compiler v.XX.1, but for v.XX.2 it might not. One of the programmers may use a new feature and Colobot may not compile with lib v.YY. Let's be honest -- it's too hard to provide new packages every time a new version of *something* comes up, at least for us, developers. You can check for yourself if current package works, it probably not.

Here comes MSYS2. It provides everything that MSYS has and much more. It is basically a whole Linux-style development environment for Windows. Most importantly, it gives you a package manager, which makes not only installation of all the needed stuff easy, but also can take care of updating it with little human effort.

Also, with the broken package, if you don't want to manually compile and install all the libraries, MSYS2 might be the only way to go.

Guide
-----

Compiling in Windows is harder than in most Linux distributions, but with MSYS2 it isn't impossible.

### Setting Up an Environment

Steps in this section needs to be done only once. After this, you can basically develop Colobot the same way you would do it normally (well, there might be a problem with integrating Windows-only IDEs).

#### Installation of MSYS2

Read this page really carefully: <https://www.msys2.org> . By following it, you should install and configure MSYS2 without major problems.

#### TIP
When you face any problems during this guide or anywhere in the future, the first thing you should do is to close all MSYS2 processes and run `autorebase.bat` script (it's in the main MSYS2 folder), especially after installation and updating. If you don't do it, odd problems might appear, like CMake not detecting GCC.

#### Running

Now you need to install `MinGW-w64 toolchain`. Open `MSYS2 Shell` and enter the following command:

```sh
pacman -S mingw-w64-i686-toolchain
```

**Warning:** Commands shown in this guide are for 32-bit operating system. If you have 64-bit OS, you must replace all `i686` occurrences with `x86_64`.

MSYS2 creates a new environment (with all the "system" variables set properly) during this installation. You have done that from default `MSYS2 Shell`. To work with GOLD, you need to switch. There are two ways of "opening" an environment you can work in:

1. Open MSYS2 Shell and enter

```sh
export PATH=/mingw32/bin:$PATH
```

or

```sh
export PATH=/mingw64/bin:$PATH
```

2. Open `MinGW-w64 Win32 Shell` (or `MinGW-w64 Win64 Shell`)

You must do one of these two steps every time you want to compile GOLD.

#### TIP
You can add "Open MSYS2 Shell here" to the context menu using registry. Save the following code as `.reg` file and run it.

```
Windows Registry Editor Version 5.00
[HKEY_CLASSES_ROOT\Directory\Background\shell\open_msys2]
@="Open MSYS2 here"
[HKEY_CLASSES_ROOT\Directory\Background\shell\open_msys2\command]
@="c:\\msys32\\usr\\bin\\mintty.exe /bin/sh -lc 'cd \"$(cygpath \"%V\")\"; export PATH=\"/mingw32/bin:$PATH\"; exec bash'"
[HKEY_CLASSES_ROOT\Folder\shell\open_msys2]
@="Open MSYS2 here"
[HKEY_CLASSES_ROOT\Folder\shell\open_msys2\command]
@="c:\\msys32\\usr\\bin\\mintty.exe /bin/sh -lc 'cd \"$(cygpath \"%V\")\"; export PATH=\"/mingw32/bin:$PATH\"; exec bash'"
```

Remember to modify the paths before you use it so they fit in your installation.

#### Installation of Tools and Dependencies

To compile Colobot you need:

- cmake
- git
- make
- gcc

Install them:

```sh
pacman -S msys2-devel git make mingw-w64-i686-cmake mingw-w64-i686-gcc
```

It's a good time to configure git or even ssh if you plan to push to the remote repository.

When you are done, you can finally get the greatest benefit of using MSYS2. You are going to install required 3rd party libraries. This is how you do it:

1. Find names of all required dependencies. They are listed in the [main INSTALL.md file](/INSTALL.md#compiling-on-linux)

2. Find each library using

```sh
pacman -Ss dependency-name
```

3. Install each library using

```sh
pacman -S package-name
```

Easy, isn't it?

If you are lazy, you can just use this one-line command, although there is no guarantee it will work or install everything (might be out of date):

```sh
pacman -S mingw-w64-i686-glew mingw-w64-i686-libpng gettext mingw-w64-i686-gettext mingw-w64-i686-libpng mingw-w64-i686-libsndfile mingw-w64-i686-libvorbis mingw-w64-i686-libogg mingw-w64-i686-openal mingw-w64_i686-physfs mingw-w64-i686-SDL2 mingw-w64-i686-SDL2_image mingw-w64-i686-SDL2_ttf
```

You should now have everything set up and working. You can close all instances of MSYS2 and autorebase to ensure everything installed properly.

### Compilation of GOLD

You could say that you have a small Linux inside of your Windows right now. To compile GOLD just follow the instructions for Linux that are available in the repository (https://github.com/colobot/colobot/blob/dev/INSTALL.md\#compiling-on-linux).

**Warning:** You must add `-G"MSYS Makefiles"` argument to `cmake`. For example, when you want to build a developer version:

```sh
cmake -DCMAKE_BUILD_TYPE=Debug -G"MSYS Makefiles" ..
```

### Dlls

Your executable should run fine if you run it from the shell (like `./colobot` in a folder with a compiled binary). However, it will rather not run if you run it "from Windows", like by double clicking the shortcut. You will get error telling you that some dlls are missing. It's normal on Windows, so don't panic. Linker do dynamic linking by default, so binary must be distributed with the libraries stored in binary dll files. You can provide them in a few ways.

#### PATH

Add `C:\msys32\mingw32\bin` to your environment variable `PATH`.

RMB on Computer -&gt; Click Properties -&gt; Advanced system settings -&gt; Environment Variables -&gt; Edit Path

Be careful though. If you have any other installation of MinGW or other tools like FPC, git and so on, this might result in conflicts (for example two `gcc.exe` files) and the effects are unpredictable.

You can use `PATH` also in other way: just copy all the dlls from `C:\msys32\mingw32\bin` to a place that's already in the `PATH`. This might result in a mess though, because you will mix files for GOLD with something else.

#### Copy

It's the way how it's done in most applications distributed for Windows. Just copy all the needed dlls to a folder with the game. All of them are in `C:\msys32\mingw32\bin`.

You can simply try to run a game and each time it gives you an error copy a missing dll to folder with an executable (like `C:\Program Files\colobot`). Or, you can use this smart command (thanks @krzys-h!):

```sh
ldd colobot.exe | grep -v /c/WINDOWS/ | cut -d ' ' -f 3 | while read -r line; do cp $line /c/path/to/the/game; done
```

#### Static Linking

You can try to compile GOLD with static linking. Nobody have done that (at least in Windows) yet, but it's possible in theory. If you did it, please, let us know!

The End
-------

Hope, this guide helped you with using Windows as a development environment especially for Colobot.

This method was first used and described by @MrSimbax. Script for gaining all the needed dlls was written by @krzys-h.
