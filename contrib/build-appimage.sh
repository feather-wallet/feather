#!/bin/bash

set -e

function verify_hash() {
    local file=$1 expected_hash=$2
    actual_hash=$(sha256sum $file | awk '{print $1}')
    if [ "$actual_hash" == "$expected_hash" ]; then
        return 0
    else
        echo "$file $actual_hash (unexpected hash)" >&2
        rm "$file"
        exit 1
    fi
}

function download_if_not_exist() {
    local file_name=$1 url=$2
    if [ ! -e $file_name ] ; then
        wget -q -O $file_name "$url"
    fi
}

APPDIR="$PWD/feather.AppDir"
mkdir -p "$APPDIR"
mkdir -p "$APPDIR/usr/share/applications/"
mkdir -p "$APPDIR/usr/bin"

echo "Downloading dependencies"

download_if_not_exist "feather.zip" "https://build.featherwallet.org/files/linux-release/$BRANCH/$FN"
unzip -q feather.zip

cp "$PWD/src/assets/feather.desktop" "$APPDIR/usr/share/applications/feather.desktop"
cp "$PWD/src/assets/images/appicons/64x64.png" "$APPDIR/feather.png"
cp "$PWD/feather" "$APPDIR/usr/bin/feather"

/appimagetool deploy "$APPDIR/usr/share/applications/feather.desktop"
VERSION=1.0 /appimagetool ./feather.AppDir

