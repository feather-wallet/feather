#!/usr/bin/env bash
set -ex

mkdir -p /output/debian
cd /output/debian

#.
#├── control.tar.gz
cp /feather/contrib/debian/control .
sed -i "s/VERSION/${VERSION}/" control
touch --no-dereference --date="@${SOURCE_DATE_EPOCH}" control
tar --owner=0 --group=0 -czvf control.tar.gz control
rm control

#├── data.tar.gz
mkdir data
cd data

mkdir -p usr/bin
cd usr/bin
# copy feather binary
cp /feather-bin feather
cd ../..

mkdir -p usr/share/applications
cp /feather/src/assets/feather.desktop usr/share/applications/

mkdir -p usr/share/icons/hicolor/128x128/apps
cp /feather/src/assets/images/appicons/128x128.png usr/share/icons/hicolor/128x128/apps/feather.png

find . -print0 | xargs -0r touch --no-dereference --date="@${SOURCE_DATE_EPOCH}"
find . | sort | tar --owner=0 --group=0 -czvf ../data.tar.gz -T -

cd /output/debian
chmod -R 755 data
rm -rf data

#└── debian-binary
echo "2.0" > debian-binary

ar r "feather_${VERSION}-1_amd64.deb" debian-binary control.tar.gz data.tar.gz
rm debian-binary control.tar.gz data.tar.gz