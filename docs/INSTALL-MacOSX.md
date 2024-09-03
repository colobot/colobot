# Compile and install instructions on MacOSX

To compile Colobot on MacOS X, you need to first get Developer Command Line Tools for OS X, which you can get for example from [Apple Developer website](https://developer.apple.com/xcode/downloads/).

After installing Developer Command Line Tools, you should have basic tools like clang and git installed. After that, you can grab other required packages with Homebrew. So as in instructions on [the project page](http://brew.sh/):
```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```
And then:
```bash
  brew install cmake sdl2 sdl2_image sdl2_ttf glew physfs flac libsndfile libvorbis vorbis-tools gettext libicns librsvg wget xmlstarlet glm
```
Gettext is installed in separate directory without adding the files to system path, so in order to get it working normally, you should call also:
```bash
  brew link gettext --force
```

If you've installed everything correctly, the simple way of compiling Colobot with CMake should work:
```bash
  git clone --recursive https://github.com/colobot/colobot.git
  mkdir colobot/build
  cd colobot/build
  cmake -DOPENAL_LIBRARY=/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/System/Library/Frameworks/OpenAL.framework/OpenAL.tbd ../
  make
```

You can then build a Colobot drag-n-drop package
```bash
  sudo make package
```
And open the package:
```bash
  open colobot-*.dmg
```
Once opened, drag the Colobot application and drop it in the Application directory. Then just launch it as any other application.
