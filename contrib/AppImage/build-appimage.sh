#!/bin/bash

set -e
unset SOURCE_DATE_EPOCH

APPDIR="$PWD/feather.AppDir"

mkdir -p "$APPDIR"
mkdir -p "$APPDIR/usr/share/applications/"
mkdir -p "$APPDIR/usr/bin"
mkdir -p "$APPDIR/usr/lib"
mkdir -p "$APPDIR/usr/plugins"

cp "src/assets/feather.desktop" "$APPDIR/usr/share/applications/feather.desktop"
cp "src/assets/images/appicons/64x64.png" "$APPDIR/feather.png"
cp "build/bin/feather" "$APPDIR/usr/bin/feather"
chmod +x "$APPDIR/usr/bin/feather"

export LD_LIBRARY_PATH=/feather/contrib/depends/x86_64-linux-gnu/lib/:/gnu/store:/gnu/store/yk91cxchassi5ykxsyd4vci32vncgjkf-gcc-cross-x86_64-linux-gnu-10.3.0-lib/x86_64-linux-gnu/lib

# fuck you linuxdeployqt
mkdir -p "$APPDIR/usr/share/doc/libc6"
touch "$APPDIR/usr/share/doc/libc6/copyright"

linuxdeployqt feather.AppDir/usr/share/applications/feather.desktop -verbose=2 -bundle-non-qt-libs -unsupported-allow-new-glibc

rm "$APPDIR/AppRun"
cp "contrib/AppImage/AppRun" "$APPDIR/"
chmod +x "$APPDIR/AppRun"

find feather.AppDir/ -exec touch -h -a -m -t 202101010100.00 {} \;

# Manually create AppImage (reproducibly)

mksquashfs feather.AppDir feather.squashfs -info -root-owned -no-xattrs -noappend -fstime 0
# mksquashfs writes a timestamp to the header
printf '\x00\x00\x00\x00' | dd conv=notrunc of=feather.squashfs bs=1 seek=$((0x8))

rm -f feather.AppImage
## Don't hardcode this
cat /feather/contrib/depends/x86_64-linux-gnu/runtime-x86_64 >> feather.AppImage
cat feather.squashfs >> feather.AppImage
chmod a+x feather.AppImage
