#!/usr/bin/env bash
set -ex

mkdir -p /output/debian
cd /output/debian

#.
#├── control.tar.gz
cp /feather/contrib/debian/control .
sed -i "s/VERSION/${VERSION}/" control
touch --no-dereference --date="@${SOURCE_DATE_EPOCH}" control
tar -czvf control.tar.gz control
rm control

#├── data.tar.gz
mkdir data
cd data

mkdir -p usr/bin
cd usr/bin
# copy feather binary
cp /feather-bin feather
cd ../..

find . -print0 | xargs -0r touch --no-dereference --date="@${SOURCE_DATE_EPOCH}"
tar -czvf ../data.tar.gz .

cd /output/debian
chmod -R 755 data
rm -rf data

#└── debian-binary
echo "2.0" > debian-binary

ar r "feather_${VERSION}-1_amd64.deb" debian-binary control.tar.gz data.tar.gz
rm debian-binary control.tar.gz data.tar.gz