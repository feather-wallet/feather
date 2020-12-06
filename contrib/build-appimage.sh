#!/bin/bash

set -e

APPDIR="$PWD/feather.AppDir"
mkdir -p "$APPDIR"
mkdir -p "$APPDIR/usr/share/applications/"
mkdir -p "$APPDIR/usr/bin"

unzip -q feather.zip

cp "$PWD/src/assets/feather.desktop" "$APPDIR/usr/share/applications/feather.desktop"
cp "$PWD/src/assets/images/appicons/64x64.png" "$APPDIR/feather.png"
cp "$PWD/feather" "$APPDIR/usr/bin/feather"

/appimagetool deploy "$APPDIR/usr/share/applications/feather.desktop"
VERSION=1.0 /appimagetool ./feather.AppDir

