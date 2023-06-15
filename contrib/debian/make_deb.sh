#!/usr/bin/env bash
set -ex

get_store_path() {
    find gnu/store -maxdepth 1 -type d -name "*$1*" | sort | head -n 1
}

mkdir -p /output/debian
cd /output/debian

#.
#├── control.tar.gz
cp /feather/contrib/debian/control .
sed -i "s/VERSION/${VERSION}/" control
tar -czvf control.tar.gz control
rm control

#├── data.tar.gz
mkdir data
cd data
tar xf /rpack .

GUIX_PROFILE=$(get_store_path "profile")

mkdir -p opt/deb-packs/feather
mv gnu opt/deb-packs/feather/

mkdir -p usr/bin
ln -s "/opt/deb-packs/feather/${GUIX_PROFILE}/bin/feather" usr/bin/feather

tar -czvf ../data.tar.gz .

cd /output/debian
chmod -R 755 data
rm -rf data

#└── debian-binary
echo "2.0" > debian-binary

ar r "feather_${VERSION}-1_amd64.deb" debian-binary control.tar.gz data.tar.gz
rm debian-binary control.tar.gz data.tar.gz