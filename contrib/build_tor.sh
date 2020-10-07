#!/usr/bin/env bash
# this file is used by feather's CMake
# arguments: ./build.tor $TAG $ROOT_FEATHER_DIR

set -ex

ERR_WIN="This script does not work on Windows"
if [[ "$OSTYPE" == "msys" ]]; then
    echo "$ERR_WIN"
    exit 1
elif [[ "$OSTYPE" == "win32" ]]; then
    echo "$ERR_WIN"
    exit 1
fi

TOR_TAG="$1"
ROOT_DIR="$2"
STATIC="$3"
TOR_DIR="$ROOT_DIR/contrib/tor"
TORSOCKS_DIR="$ROOT_DIR/contrib/torsocks"
TARGET_DIR="$ROOT_DIR/src/tor"

CPU_CORE_COUNT="$(nproc)"

#
### tor
#

pushd "$TOR_DIR"

rm -rf "$TOR_DIR/build"
mkdir -p "$TOR_DIR/build"

# configure
git -C "$TOR_DIR" fetch
git -C "$TOR_DIR" checkout tor-0.4.3.5
bash "$TOR_DIR/autogen.sh"

if [[ "$STATIC" = "ON" ]]; then
    # static assumes that openssl has been compiled with:
    #    CFLAGS='-fPIC' CXXFLAGS='-fPIC' ./config no-asm no-shared no-zlib-dynamic --prefix=/usr/local/openssl --openssldir=/usr/local/openssl
    # and libevent with:
    #    cmake -DEVENT_LIBRARY_STATIC=ON -DOPENSSL_ROOT_DIR=/usr/local/openssl -DCMAKE_INSTALL_PREFIX=/usr/local/libevent
    # and zlib with:
    #    CFLAGS='-fPIC' CXXFLAGS='-fPIC' ./configure --static --prefix=/usr/local/zlib

    LDFLAGS="-L/usr/local/openssl/lib/" LIBS="-lssl -lcrypto -lpthread -ldl" CPPFLAGS="-I/usr/local/openssl/include/" ./configure \
    --enable-static-zlib \
    --enable-static-openssl \
    --enable-static-libevent \
    --disable-system-torrc \
    --with-libevent-dir=/usr/local/libevent \
    --with-openssl-dir=/usr/local/openssl/ \
    --with-zlib-dir=/usr/local/zlib \
    --disable-system-torrc \
    --disable-tool-name-check \
    --disable-systemd \
    --disable-lzma \
    --disable-unittests \
    --disable-zstd \
    --disable-seccomp \
    --disable-asciidoc \
    --disable-manpage \
    --disable-html-manual \
    --disable-system-torrc \
    --prefix="$TOR_DIR/build"
else
    bash "$TOR_DIR/configure" \
    --disable-tool-name-check \
    --disable-systemd \
    --disable-lzma \
    --disable-unittests \
    --disable-zstd \
    --disable-asciidoc \
    --disable-manpage \
    --disable-html-manual \
    --prefix="$TOR_DIR/build"
fi

# build
make -j "$CPU_CORE_COUNT"
make install -j "$CPU_CORE_COUNT"

# copy to lib/tor
cp "$TOR_DIR/build/bin/tor" "$TARGET_DIR"
cp "$TOR_DIR/build/etc/tor/torrc.sample"* "$TARGET_DIR"

#
### torsocks
#

pushd "$TORSOCKS_DIR"
mkdir -p "$TORSOCKS_DIR/build"

# configure
bash "$TORSOCKS_DIR/autogen.sh"
bash "$TORSOCKS_DIR/configure" --prefix="$TORSOCKS_DIR/build"

# build
make -j "$CPU_CORE_COUNT"
make install -j "$CPU_CORE_COUNT"

# copy to lib/torsocks
cp "$TORSOCKS_DIR/build/lib/torsocks/"* "$TARGET_DIR"
cp "$TORSOCKS_DIR/build/bin/"* "$TARGET_DIR"
cp "$TORSOCKS_DIR/build/etc/tor/"* "$TARGET_DIR"

#
### verify installation
#

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    for fn in "$TARGET_DIR/libtorsocks.so" "$TARGET_DIR/tor"; do
        if [[ ! -f "$fn" ]]; then
            echo "[*] Failed to install tor or torsocks: no such file $fn"
            exit 1
        fi; done
elif [[ "$OSTYPE" == "darwin"* ]]; then
    for fn in "$TARGET_DIR/libtorsocks.dylib" "$TARGET_DIR/tor"; do
        if [[ ! -f "$fn" ]]; then
            echo "[*] Failed to install tor or torsocks: no such file $fn"
            exit 1
        fi; done
fi

echo "[*] Compiled tor/torsocks into $TARGET_DIR"
