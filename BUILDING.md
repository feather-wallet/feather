# Buildbot builds

The docker build bins can be found here: https://build.featherwallet.org/files/

## Docker static builds

Static builds via Docker are done in 3 steps:

1. Cloning this repository (+submodules)
2. Creating a base Docker image
3. Using the base image to compile a build

### Windows

The docker image for Windows static compiles uses Ubuntu 18.04 and installs [mxe](https://mxe.cc) from [our git](https://git.wownero.com/feather/mxe/src/branch/feather-patch), 
which comes with: OpenSSL 1.1.1g, Qt 5.15.0 (OpenGL via mesa). For more information, check the Dockerfile: `Dockerfile_windows`.

#### 1. Clone

```bash
git clone --recursive https://git.wownero.com/feather/feather.git
cd feather
```

#### 2. Base image

Warning: Building the MXE base image takes up to a hour, so go watch a movie.

```bash
docker build -f Dockerfile_windows --tag feather:win --build-arg THREADS=8 .
```

Note: You only need to build the base image once.

#### 3. Build

```bash
docker run --rm -it -v /tmp/ccache:/root/.ccache -v /root/feather:/feather -w /feather feather:win /bin/bash -c 'PATH="/mxe/usr/bin/:$PATH" TOR="/mxe/usr/x86_64-w64-mingw32.static/bin/tor.exe" XMRIG="/xmrig/xmrig.exe" make windows-mxe-release -j8'
```

Replace `PATH_TO_FEATHER` with the absolute path to Feather locally. 

The resulting binary can be found in `build/bin/feather.exe`.

### Linux

The docker image for Linux static compiles uses Ubuntu 18.04 and compiles the required libraries statically so that 
the resulting Feather binary is static. It comes with OpenSSL 1.1.1g, Qt 5.15.0 (OpenGL disabled). For more information, 
check the Dockerfile: `Dockerfile`.

#### 1. Clone

```bash
git clone --recursive https://git.wownero.com/feather/feather.git
cd feather
```

#### 2. Base image

Warning: Building the base image takes a while, go prepare some dinner.

```bash
docker build --tag feather:linux --build-arg THREADS=8 .
```

Note: You only need to build the base image once.

#### 3. Build

```bash
docker run --env OPENSSL_ROOT_DIR=/usr/local/openssl/ --rm -it -v /tmp/ccache:/root/.ccache -v PATH_TO_FEATHER:/feather -w /feather feather:linux sh -c 'TOR="/usr/local/tor/bin/tor" XMRIG="/xmrig/xmrig" make release-static -j8'
```

Replace `PATH_TO_FEATHER` with the absolute path to Feather locally.

The resulting binary can be found in `build/bin/feather`.

## macOS

For MacOS it's easiest to leverage [brew](https://brew.sh) to install the required dependencies. 

```bash
HOMEBREW_OPTFLAGS="-march=core2" HOMEBREW_OPTIMIZATION_LEVEL="O0" \
    brew install boost zmq openssl libpgm miniupnpc libsodium expat libunwind-headers protobuf libgcrypt qrencode ccache cmake pkgconfig git
```

Clone the repository.

```bash
git clone --recursive https://git.wownero.com/feather/feather.git
``` 

Get the latest LTS from here: https://www.qt.io/offline-installers and install.

Build Feather.

```bash
CMAKE_PREFIX_PATH=~/Qt5.15.1/5.15.1/clang_64 make mac-release
```

The resulting Mac OS application can be found `build/bin/feather.app` and will **not** have Tor embedded.