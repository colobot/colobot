#!/bin/bash -e
if [ -z "$ANDROID_NDK" ]; then
    echo "Please set the path to Android NDK in \$ANDROID_NDK"
    exit 1
fi
if [ -z "$ANDROID_PLATFORM_LEVEL" ] || [ -z "$ANDROID_ABI" ]; then
    echo "Please \$ANDROID_ABI and \$ANDROID_PLATFORM_LEVEL"
    exit 1
fi

if [ "$ANDROID_ABI" == "armeabi-v7a" ]; then
  export ANDROID_TOOLCHAIN_NAME="armv7a-linux-androideabi"
elif [ "$ANDROID_ABI" == "arm64-v8a" ]; then
  export ANDROID_TOOLCHAIN_NAME="aarch64-linux-android"
elif [ "$ANDROID_ABI" == "x86" ]; then
  export ANDROID_TOOLCHAIN_NAME="i686-linux-android"
elif [ "$ANDROID_ABI" == "x86_64" ]; then
  export ANDROID_TOOLCHAIN_NAME="x86_64-linux-android"
else
  echo "Invalid Android ABI: ${ANDROID_ABI}."
  exit 1
fi


export WORKDIR="$PWD/$ANDROID_ABI-$ANDROID_PLATFORM_LEVEL/work"
export SYSROOT="$PWD/$ANDROID_ABI-$ANDROID_PLATFORM_LEVEL/sysroot"
rm -rf $WORKDIR $SYSROOT
mkdir -p $WORKDIR $SYSROOT
cd $WORKDIR

export AR=$ANDROID_NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-ar
export CC=$ANDROID_NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/$ANDROID_TOOLCHAIN_NAME$ANDROID_PLATFORM_LEVEL-clang
export AS=$CC
export CXX=$ANDROID_NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/$ANDROID_TOOLCHAIN_NAME$ANDROID_PLATFORM_LEVEL-clang++
export LD=$ANDROID_NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/ld
export RANLIB=$ANDROID_NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-ranlib
export STRIP=$ANDROID_NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-strip

# Don't mix up .pc files from your host and build target
export PKG_CONFIG_PATH=$SYSROOT/usr/lib/pkgconfig

wget http://prdownloads.sourceforge.net/libpng/libpng-1.6.37.tar.gz
tar xf libpng-1.6.37.tar.gz
cd libpng-1.6.37
./configure --build x86_64 --host $ANDROID_TOOLCHAIN_NAME --prefix $SYSROOT/usr --with-sysroot $SYSROOT --disable-shared --enable-static
make -j$(nproc)
make install
cd ..

wget https://download.savannah.gnu.org/releases/freetype/freetype-2.10.4.tar.gz
tar xf freetype-2.10.4.tar.gz
cd freetype-2.10.4
./configure --build x86_64 --host $ANDROID_TOOLCHAIN_NAME --prefix $SYSROOT/usr --with-sysroot $SYSROOT --disable-shared --enable-static --without-brotli --without-harfbuzz
make -j$(nproc)
make install
cd ..

wget https://libsdl.org/release/SDL2-2.0.14.tar.gz
tar xf SDL2-2.0.14.tar.gz
cd SDL2-2.0.14
./configure --build x86_64 --host $ANDROID_TOOLCHAIN_NAME --prefix $SYSROOT/usr --with-sysroot $SYSROOT --disable-shared --enable-static --disable-video-wayland --disable-pulseaudio
make -j$(nproc)
make install
cd ..

wget https://libsdl.org/projects/SDL_image/release/SDL2_image-2.0.5.tar.gz
tar xf SDL2_image-2.0.5.tar.gz
cd SDL2_image-2.0.5
# disable the stupid test program
sed -i 's/noinst_PROGRAMS/#noinst_PROGRAMS/g' Makefile.am
./autogen.sh
./configure --build x86_64 --host $ANDROID_TOOLCHAIN_NAME --prefix $SYSROOT/usr --with-sysroot $SYSROOT --disable-shared --enable-static --disable-webp --disable-sdltest
make -j$(nproc)
make install
cd ..

wget https://libsdl.org/projects/SDL_ttf/release/SDL2_ttf-2.0.15.tar.gz
tar xf SDL2_ttf-2.0.15.tar.gz
cd SDL2_ttf-2.0.15
sed -i 's/noinst_PROGRAMS/#noinst_PROGRAMS/g' Makefile.am
./autogen.sh
./configure --build x86_64 --host $ANDROID_TOOLCHAIN_NAME --prefix $SYSROOT/usr --with-sysroot $SYSROOT --disable-shared --enable-static
make -j$(nproc)
make install
cd ..

wget https://ftp.osuosl.org/pub/xiph/releases/ogg/libogg-1.3.4.tar.gz
tar xf libogg-1.3.4.tar.gz
cd libogg-1.3.4
./configure --build x86_64 --host $ANDROID_TOOLCHAIN_NAME --prefix $SYSROOT/usr --with-sysroot $SYSROOT --disable-shared --enable-static
make -j$(nproc)
make install
cd ..

wget https://ftp.osuosl.org/pub/xiph/releases/vorbis/libvorbis-1.3.7.tar.gz
tar xf libvorbis-1.3.7.tar.gz
cd libvorbis-1.3.7
sed -i 's/-mno-ieee-fp//g' configure.ac
./autogen.sh
./configure --build x86_64 --host $ANDROID_TOOLCHAIN_NAME --prefix $SYSROOT/usr --with-sysroot $SYSROOT --disable-shared --enable-static
make -j$(nproc)
make install
cd ..

wget https://ftp.osuosl.org/pub/xiph/releases/flac/flac-1.3.3.tar.xz
tar xf flac-1.3.3.tar.xz
cd flac-1.3.3
./configure --build x86_64 --host $ANDROID_TOOLCHAIN_NAME --prefix $SYSROOT/usr --with-sysroot $SYSROOT --disable-shared --enable-static
make -j$(nproc)
make install
cd ..

wget http://www.mega-nerd.com/libsndfile/files/libsndfile-1.0.28.tar.gz
tar xf libsndfile-1.0.28.tar.gz
cd libsndfile-1.0.28
# which part of "AM_CPPFLAGS is ignored in preference to a per-executable (or per-library) _CPPFLAGS variable if it is defined." was too hard to understand for the person who wrote this?!
sed -i 's/libsndfile_la_CPPFLAGS =/libsndfile_la_CPPFLAGS = $(AM_CPPFLAGS)/g' src/Makefile.am
aclocal-1.15
./configure --build x86_64 --host $ANDROID_TOOLCHAIN_NAME --target $ANDROID_TOOLCHAIN_NAME --prefix $SYSROOT/usr --with-sysroot $SYSROOT --disable-shared --enable-static
make -j$(nproc)
make install
cd ..

wget https://ftp.gnu.org/pub/gnu/libiconv/libiconv-1.16.tar.gz
tar xf libiconv-1.16.tar.gz
cd libiconv-1.16
./configure --build x86_64 --host $ANDROID_TOOLCHAIN_NAME --prefix $SYSROOT/usr --with-sysroot $SYSROOT --disable-shared --enable-static
make -j$(nproc)
make install
cd ..

wget https://ftp.gnu.org/pub/gnu/gettext/gettext-0.21.tar.gz
tar xf gettext-0.21.tar.gz
cd gettext-0.21
./configure --build x86_64 --host $ANDROID_TOOLCHAIN_NAME --prefix $SYSROOT/usr --with-sysroot $SYSROOT --disable-shared --enable-static
make -j$(nproc)
make install
cd ..

wget https://icculus.org/physfs/downloads/physfs-3.0.2.tar.bz2
tar xf physfs-3.0.2.tar.bz2
cd physfs-3.0.2
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake -DANDROID_ABI=$ANDROID_ABI -DANDROID_PLATFORM=$ANDROID_PLATFORM_LEVEL -DCMAKE_INSTALL_PREFIX=$SYSROOT/usr -DPHYSFS_BUILD_SHARED=OFF ..
make -j$(nproc)
make install
cd ../..

git clone -b 1.21.1 --depth=1 git://repo.or.cz/openal-soft.git openal-soft-1.21.1
cd openal-soft-1.21.1/build
cmake -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake -DANDROID_ABI=$ANDROID_ABI -DANDROID_PLATFORM=$ANDROID_PLATFORM_LEVEL -DCMAKE_INSTALL_PREFIX=$SYSROOT/usr ..
make -j$(nproc)
make install
cd ../..

git clone --depth=1 https://github.com/moritz-wundke/Boost-for-Android.git
cd Boost-for-Android
sed -i 's/"21.0"|"21.1"|"21.2"|"21.3")/"21.0"|"21.1"|"21.2"|"21.3"|"22.0")/g' build-android.sh
TOOLCHAIN="" AR="" CC="" AS="" CXX="" LD="" RANLIB="" STRIP="" PKG_CONFIG_PATH="" ./build-android.sh $ANDROID_NDK --boost=1.74.0 --toolchain=llvm --with-libraries=system,filesystem,regex --arch=$ANDROID_ABI
cp -r build/out/$ANDROID_ABI/lib $SYSROOT/usr/
cp -r build/out/$ANDROID_ABI/include $SYSROOT/usr/
cd ..
