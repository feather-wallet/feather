# Buildbot builds

The docker build bins can be found here: https://build.featherwallet.org/files/

## Docker static builds

Static builds via Docker are done in 3 steps:

1. Cloning this repository (+submodules)
2. Creating a base Docker image
3. Using the base image to compile a build

### Linux (reproducible)

The docker image for reproducible Linux static builds uses Ubuntu 16.04 and compiles the required libraries statically 
so that the resulting Feather binary is static. For more information, check the Dockerfile: `Dockerfile`.

#### 1. Clone

```bash
git clone --branch master --recursive https://git.wownero.com/feather/feather.git
cd feather
```

Replace `master` with the desired version tag (e.g. `beta-1`) to build the release binary.

#### 2. Base image

```bash
docker build --tag feather:linux --build-arg THREADS=4 .
```

Building the base image takes a while. You only need to build the base image once.

#### 3. Build

```bash
docker run --rm -it -v $PWD:/feather --env OPENSSL_ROOT_DIR=/usr/local/openssl/ -w /feather feather:linux sh -c 'TOR_BIN="/usr/local/tor/bin/tor" make release-static -j4'
```

If you're re-running a build make sure to `rm -rf build/` first.

The resulting binary can be found in `build/bin/feather`.

Hashes for tagged commits should match:

```
beta-1: d1a52e3bac1abbae4adda1fc88cb2a7a06fbd61085868421897c6a4f3f4eb091  feather
```

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
docker run --rm -it -v /tmp/ccache:/root/.ccache -v PATH_TO_FEATHER:/feather -w /feather feather:win /bin/bash -c 'PATH="/mxe/usr/bin/:$PATH" TOR_BIN="/mxe/usr/x86_64-w64-mingw32.static/bin/tor.exe" make windows-mxe-release -j8'
```

Replace `PATH_TO_FEATHER` with the absolute path to Feather locally.

The resulting binary can be found in `build/bin/feather.exe`.

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
