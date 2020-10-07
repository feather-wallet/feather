#!/usr/bin/env bash
# Used for macos buildbot
HASH="$1"
echo "[+] hash: $HASH"

echo "[+] Building"

rm ~/feather.zip 2>&1 >/dev/null
cd ~/feather
git fetch
git reset --hard "$HASH"
git submodule update --init --depth 50 contrib/tor
git submodule update --init --depth 50 contrib/torsocks
git submodule update --init --depth 120 monero
git submodule update --init --depth 120 --recursive monero

CMAKE_PREFIX_PATH=~/Qt/5.15.1/clang_64 make -j3 mac-release

if [[ $? -eq 0 ]]; then
    echo "[+] Feather built OK"
    cd ~/feather/build/bin
    zip -qr ~/feather.zip feather.app
else
    echo "[+] Error!"
    exit 1;
fi
