#!/bin/bash

set -e
unset SOURCE_DATE_EPOCH

# Manually create the AppImage (reproducibly) since linuxdeployqt is not able to create cross-compiled AppImages

APPDIR="$PWD/feather.AppDir"

mkdir -p "$APPDIR"
mkdir -p "$APPDIR/usr/share/applications/"
mkdir -p "$APPDIR/usr/bin"

cp "src/assets/feather.desktop" "$APPDIR/usr/share/applications/feather.desktop"
cp "src/assets/feather.desktop" "$APPDIR/feather.desktop"
cp "src/assets/images/appicons/64x64.png" "$APPDIR/feather.png"
cp "build/bin/feather" "$APPDIR/usr/bin/feather"
chmod +x "$APPDIR/usr/bin/feather"

cp "contrib/AppImage/AppRun" "$APPDIR/"
chmod +x "$APPDIR/AppRun"

find feather.AppDir/ -exec touch -h -a -m -t 202101010100.00 {} \;

mksquashfs feather.AppDir feather.squashfs -comp zstd -info -root-owned -no-xattrs -noappend -fstime 0
# mksquashfs writes a timestamp to the header
printf '\x00\x00\x00\x00' | dd conv=notrunc of=feather.squashfs bs=1 seek=$((0x8))

rm -f feather.AppImage

cat /feather/contrib/depends/${HOST}/runtime >> feather.AppImage
cat feather.squashfs >> feather.AppImage
chmod a+x feather.AppImage
