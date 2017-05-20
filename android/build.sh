#!/bin/sh
mkdir -p assets
mkdir -p pak
cp -r jni/Parabot/addons pak/
python2 makepak.py pak/ assets/extras.pak
ndk-build NDK_TOOLCHAIN_VERSION=4.8 NDK_DEBUG=0 V=1 -j5 APP_CFLAGS="-w"
ant release
#jarsigner -verbose -sigalg SHA1withRSA -digestalg SHA1 -keystore ../myks.keystore bin/mod-unsigned.apk xashdroid -tsa https://timestamp.geotrust.com/tsa
#zipalign 4 bin/cs16-client-unsigned.apk bin/mod.apk
