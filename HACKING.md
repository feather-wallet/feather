# Documentation for developers

Feather is developed primarily on Linux. It uses Qt 5.15.* and chances are that your 
distro's package manager has a lower version. It is therefore recommended that you install 
Qt manually using the online installer, which can be found here: https://www.qt.io/download 
(under open-source).

## Jetbrains Clion

Feather was developed using JetBrains Clion since it integrates nicely 
with CMake and comes with a built-in debugger. To pass CMake flags to CLion, 
go to `File->Settings->Build->CMake`, set Build Type to `Debug` and set your 
preferred CMake options/definitions.

## Requirements

### Ubuntu/Debian

```bash
apt install -y git cmake libqrencode-dev build-essential cmake libboost-all-dev \
miniupnpc libunbound-dev graphviz doxygen libunwind8-dev pkg-config libssl-dev \
libzmq3-dev libsodium-dev libhidapi-dev libnorm-dev libusb-1.0-0-dev libpgm-dev \
libprotobuf-dev protobuf-compiler libgcrypt20-dev
```

## Mac OS

```bash
brew install boost zmq openssl libpgm miniupnpc libsodium expat libunwind-headers \
protobuf libgcrypt qrencode ccache cmake pkgconfig git
```

## CMake

After installing Qt you might have a folder called `/home/$user/Qt/`. You need to pass this to CMake 
via the `CMAKE_PREFIX_PATH` definition. For me this is:

```
-DCMAKE_PREFIX_PATH=/home/dsc/QtNew/5.15.0/gcc_64
```

There are some Monero/Feather related options/definitions that you may pass:

- `-DXMRIG=OFF` - disable XMRig feature
- `-DTOR_BIN=/path/to/tor` - Embed a Tor executable inside Feather
- `-DDONATE_BEG=OFF` - disable the dreaded donate requests

And:

```
-DMANUAL_SUBMODULES=1  
-DUSE_DEVICE_TREZOR=OFF 
-DUSE_SINGLE_BUILDDIR=ON 
-DDEV_MODE=ON 
```

If you have OpenSSL installed in a custom location, try:

```
-DOPENSSL_INCLUDE_DIR=/usr/local/lib/openssl-1.1.1g/include 
-DOPENSSL_SSL_LIBRARY=/usr/local/lib/openssl-1.1.1g/libssl.so.1.1 
-DOPENSSL_CRYPTO_LIBRARY=/usr/local/lib/openssl-1.1.1g/libcrypto.so.1.1
```

I prefer also enabling verbose makefiles, which may be useful in some situations.

```
-DCMAKE_VERBOSE_MAKEFILE=ON
```

Enable debugging symbols:

```bash
-DCMAKE_BUILD_TYPE=Debug
```

## Feather

It's best to install Tor locally as a service and start Feather with `--use-local-tor`, this 
prevents the child process from starting up and saves time.

#### Ubuntu/Debian

```bash
apt install -y tor
sudo service tor start
```

#### Mac OS

```bash
brew install tor
brew services start tor
```

To skip the wizards and open a wallet directly use `--wallet-file`: 

```bash
./feather --use-local-tor --wallet-file /home/user/Monero/wallets/bla.keys
```

It is recommended that you use `--stagenet` for development. Testnet is also possible, 
but you'll have to provide Feather a testnet node of your own.
 
