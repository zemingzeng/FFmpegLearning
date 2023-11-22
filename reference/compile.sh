#!/bin/sh

#ndk tool path
NDK=/home/gms/zzm/NDK/android-ndk-r24

#64:arm64-v8a-->aarch64,32:armeabi-v7a-->armv7a
ARCH=aarch64

#min support api level(Api23->Android6.0,Api34->Android14.0)
API=23

#compile libs output path
OUTPUT=$(pwd)/android/out/arm64-v8a/ 

#ndk cross compile tool path
TOOLCHAIN=/home/gms/zzm/NDK/android-ndk-r24/toolchains/llvm/prebuilt/linux-x86_64 

compile() {
  ./configure \
  --target-os=android \
  --prefix=$OUTPUT \
  --arch=$ARCH \
  --sysroot=$TOOLCHAIN/sysroot \
  --disable-static \
  --disable-ffmpeg \
  --disable-ffplay \
  --disable-ffprobe \
  --disable-debug \
  --disable-doc \
  --disable-avdevice \
  --enable-shared \
  --enable-cross-compile \
  --cross-prefix=$TOOLCHAIN/bin/llvm- \
  --cc=$TOOLCHAIN/bin/aarch64-linux-android$API-clang \
  --cxx=$TOOLCHAIN/bin/aarch64-linux-android$API-clang++ \
  --extra-cflags="-fpic"
  make clean all
  make -j8
  make install  
}

compile

