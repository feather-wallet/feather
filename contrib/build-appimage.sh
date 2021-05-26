#!/bin/bash

set -e
unset SOURCE_DATE_EPOCH

APPDIR="$PWD/feather.AppDir"
rm -rf $APPDIR
mkdir -p "$APPDIR"
mkdir -p "$APPDIR/usr/share/applications/"
mkdir -p "$APPDIR/usr/bin"

cp "$PWD/../src/assets/feather.desktop" "$APPDIR/usr/share/applications/feather.desktop"
cp "$PWD/../src/assets/images/appicons/64x64.png" "$APPDIR/feather.png"
cp "$PWD/bin/feather" "$APPDIR/usr/bin/feather"

LD_LIBRARY_PATH=/usr/local/lib /linuxdeployqt/squashfs-root/AppRun feather.AppDir/usr/share/applications/feather.desktop -bundle-non-qt-libs

find feather.AppDir/ -exec touch -h -a -m -t 202101010100.00 {} \;

# Manually create AppImage (reproducibly)

# download runtime
wget -nc https://github.com/AppImage/AppImageKit/releases/download/12/runtime-x86_64
echo "24da8e0e149b7211cbfb00a545189a1101cb18d1f27d4cfc1895837d2c30bc30 runtime-x86_64" | sha256sum -c

mksquashfs feather.AppDir feather.squashfs -info -root-owned -no-xattrs -noappend -fstime 0
# mksquashfs writes a timestamp to the header
printf '\x00\x00\x00\x00' | dd conv=notrunc of=feather.squashfs bs=1 seek=$((0x8))

rm -f feather.AppImage
cat runtime-x86_64 >> feather.AppImage
cat feather.squashfs >> feather.AppImage
chmod a+x feather.AppImage
