#!/bin/bash -e
if [ -z "$ANDROID_NDK" ]; then
    echo "Please set the path to Android NDK in \$ANDROID_NDK"
    exit 1
fi
export ANDROID_PLATFORM_LEVEL="21"  # this should match the min api level in AndroidManifest.xml
#for ANDROID_ABI in armeabi-v7a arm64-v8a x86 x86_64; do
for ANDROID_ABI in x86 x86_64; do
    export ANDROID_ABI
    ./build_deps.sh
done
