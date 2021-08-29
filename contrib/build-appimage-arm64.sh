#!/bin/bash

# TODO: Merge with build-appimage.sh

set -e
unset SOURCE_DATE_EPOCH

# Temporary workaround for linuxdeployqt issue on arm64
if [ "$(uname -m)" = "aarch64" ]; then
  pushd /
  apt update
  apt install -y qt5-default
  git clone https://github.com/probonopd/linuxdeployqt.git
  cd linuxdeployqt
  git reset --hard b4697483c98120007019c3456914cfd1dba58384
  qmake
  make -j$THREADS
  make install
  rm -rf $(pwd)
  popd
fi

APPDIR="$PWD/feather.AppDir"
rm -rf $APPDIR
mkdir -p "$APPDIR"
mkdir -p "$APPDIR/usr/share/applications/"
mkdir -p "$APPDIR/usr/bin"
mkdir -p "$APPDIR/usr/lib"
mkdir -p "$APPDIR/usr/plugins"

cp "$PWD/../src/assets/feather.desktop" "$APPDIR/usr/share/applications/feather.desktop"
cp "$PWD/../src/assets/images/appicons/64x64.png" "$APPDIR/feather.png"
cp "$PWD/release/bin/feather" "$APPDIR/usr/bin/feather"
chmod +x "$APPDIR/usr/bin/feather"

export LD_LIBRARY_PATH=/usr/lib/aarch64-linux-gnu/:/usr/local/lib/$LD_LIBRARY_PATH
linuxdeployqt feather.AppDir/usr/share/applications/feather.desktop -verbose=2 -bundle-non-qt-libs

pushd feather.AppDir/usr/plugins
ln -s ../lib/ gstreamer
popd

GST_PLUGINS=("libgstcamerabin.so libgstcoreelements.so libgstvolume.so libgstapp.so libgstvideoconvert.so
              libgstvideoscale.so libgstvideo4linux2.so libgstencoding.so libgstmultifile.so libgstmatroska.so
              libgstvpx.so libgstjpegformat.so libgstjpeg.so libgstautodetect.so libgstaudiotestsrc.so
              libgstvorbis.so libgstaudiorate.so libgstaudioconvert.so libgstaudioresample.so libgstvideocrop.so")

for plugin in ${GST_PLUGINS[*]}; do
  cp /usr/lib/aarch64-linux-gnu/gstreamer-1.0/$plugin feather.AppDir/usr/plugins/gstreamer/

  # linuxdeployqt doesn't set RUNPATH on libs that are only loaded at runtime
  patchelf --set-rpath "\$ORIGIN" feather.AppDir/usr/plugins/gstreamer/$plugin
done

cp /usr/lib/aarch64-linux-gnu/gstreamer1.0/gstreamer-1.0/gst-plugin-scanner feather.AppDir/usr/plugins/gstreamer/

# Need second deploy for gstreamer dependencies
linuxdeployqt feather.AppDir/usr/share/applications/feather.desktop -verbose=2 -bundle-non-qt-libs

rm "$APPDIR/AppRun"
cp "$PWD/../contrib/AppImage/AppRun" "$APPDIR/"
chmod +x "$APPDIR/AppRun"

find feather.AppDir/ -exec touch -h -a -m -t 202101010100.00 {} \;

# Manually create AppImage (reproducibly)

# download runtime
wget -nc https://github.com/AppImage/AppImageKit/releases/download/13/runtime-aarch64
echo "d2624ce8cc2c64ef76ba986166ad67f07110cdbf85112ace4f91611bc634c96a runtime-aarch64" | sha256sum -c

mksquashfs feather.AppDir feather.squashfs -info -root-owned -no-xattrs -noappend -fstime 0
# mksquashfs writes a timestamp to the header
printf '\x00\x00\x00\x00' | dd conv=notrunc of=feather.squashfs bs=1 seek=$((0x8))

rm -f feather.AppImage
cat runtime-aarch64 >> feather.AppImage
cat feather.squashfs >> feather.AppImage
chmod a+x feather.AppImage
