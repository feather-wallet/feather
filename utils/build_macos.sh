#!/usr/bin/env bash
# Used for macos buildbot
HASH="$1"
echo "[+] hash: $HASH"

export DRONE=true
echo "[+] Building"

rm ~/feather.zip 2>&1 >/dev/null
cd ~/feather
git fetch --all
git reset --hard "$HASH"
git submodule update --init --depth 120 monero
git submodule update --init --depth 120 --recursive monero

cp "/Users/administrator/tor/libevent-2.1.7.dylib" "/Users/administrator/feather/src/assets/exec/libevent-2.1.7.dylib"
CMAKE_PREFIX_PATH="~/Qt/5.15.1/clang_64" TOR="/Users/administrator/tor/tor" XMRIG="/Users/administrator/xmrig/xmrig" make -j3 mac-release

if [[ $? -eq 0 ]]; then
    echo "[+] Feather built OK"
    cd ~/feather/build/bin
    zip -qr ~/feather.zip feather.app
else
    echo "[+] Error!"
    exit 1;
fi
