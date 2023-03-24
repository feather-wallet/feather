#!/bin/bash

set -e
unset SOURCE_DATE_EPOCH

# Manually create the AppImage (reproducibly) since linuxdeployqt is not able to create cross-compiled AppImages

APPDIR="$PWD/feather.AppDir"

mkdir -p "$APPDIR"
mkdir -p "$APPDIR/usr/share/applications/"
mkdir -p "$APPDIR/usr/bin"
mkdir -p "$APPDIR/usr/lib"

cp "src/assets/feather.desktop" "$APPDIR/usr/share/applications/feather.desktop"
cp "src/assets/feather.desktop" "$APPDIR/feather.desktop"
cp "src/assets/images/appicons/64x64.png" "$APPDIR/feather.png"
cp "build/bin/feather" "$APPDIR/usr/bin/feather"
chmod +x "$APPDIR/usr/bin/feather"

mkdir -p "${APPDIR}/usr/lib"

XCB_LIBS=(libxcb-cursor.so.0 libxcb-render.so.0 libxcb-xfixes.so.0 libxcb-icccm.so.4 libxcb-render-util.so.0
          libxcb-xkb.so.1 libxcb-image.so.0 libxcb-shape.so.0 libxkbcommon.so.0 libxcb-keysyms.so.1
          libxcb-shm.so.0 libxkbcommon-x11.so.0 libxcb-randr.so.0 libxcb-sync.so.1)

for lib in "${XCB_LIBS[@]}"
do
    echo "${lib}"
    cp "/feather/contrib/depends/${HOST}/lib/${lib}" "${APPDIR}/usr/lib/"
    "${HOST}-strip" "${APPDIR}/usr/lib/${lib}"
    patchelf --set-rpath "\$ORIGIN" "${APPDIR}/usr/lib/${lib}"
done

patchelf --set-rpath "\$ORIGIN/../lib" "$APPDIR/usr/bin/feather"

cp "contrib/AppImage/AppRun" "$APPDIR/"
chmod +x "$APPDIR/AppRun"

find feather.AppDir/ -exec touch -h -a -m -t 202101010100.00 {} \;

mksquashfs feather.AppDir feather.squashfs -info -root-owned -no-xattrs -noappend -fstime 0
# mksquashfs writes a timestamp to the header
printf '\x00\x00\x00\x00' | dd conv=notrunc of=feather.squashfs bs=1 seek=$((0x8))

rm -f feather.AppImage

cat /feather/contrib/depends/${HOST}/runtime >> feather.AppImage
cat feather.squashfs >> feather.AppImage
chmod a+x feather.AppImage
