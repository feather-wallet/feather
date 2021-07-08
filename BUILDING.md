## Buildbot builds

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
git clone --branch master --recursive https://git.featherwallet.org/feather/feather.git
cd feather
```

Replace `master` with the desired version tag (e.g. `beta-8`) to build the release binary.

#### 2. Base image

```bash
docker build --tag feather:linux --build-arg THREADS=4 .
```

Building the base image takes a while. You only need to build the base image once.

#### 3. Build

##### Standalone binary

```bash
docker run --rm -it -v $PWD:/feather -w /feather feather:linux sh -c 'CHECK_UPDATES=On make release-static -j4'
```

If you're re-running a build make sure to `rm -rf build/` first.

The resulting binary can be found in `build/bin/feather`.

##### AppImage

First create the standalone binary using the Docker command in the previous step.

```bash
docker run --rm -it -v $PWD:/feather -w /feather/build feather:linux ../contrib/build-appimage.sh
```

The resulting AppImage will be located in `./build`.

### Windows (reproducible)

#### 1. Clone

```bash
git clone --branch master --recursive https://git.featherwallet.org/feather/feather.git
cd feather
```

Replace `master` with the desired version tag (e.g. `beta-8`) to build the release binary.

#### 2. Base image


```bash
docker build -f Dockerfile.windows --tag feather:win --build-arg THREADS=4 .
```

Building the base image takes a while. You only need to build the base image once.

#### 3. Build

```bash
docker run --rm -it -v $PWD:/feather -w /feather feather:win sh -c 'CHECK_UPDATES=On make depends root=/depends target=x86_64-w64-mingw32 tag=win-x64 -j4'
```

If you're re-running a build make sure to `rm -rf build/` first.

The resulting binary can be found in `build/x86_64-w64-mingw32/release/bin/feather.exe`.

## macOS

For MacOS it's easiest to leverage [brew](https://brew.sh) to install the required dependencies. 

```bash
HOMEBREW_OPTFLAGS="-march=core2" HOMEBREW_OPTIMIZATION_LEVEL="O0" \
    brew install boost zmq openssl libpgm miniupnpc libsodium expat libunwind-headers protobuf libgcrypt qrencode ccache cmake pkgconfig git
```

Clone the repository.

```bash
git clone --recursive https://git.featherwallet.org/feather/feather.git
``` 

Get the latest LTS from here: https://www.qt.io/offline-installers and install.

Build Feather.

```bash
CMAKE_PREFIX_PATH=~/Qt5.15.1/5.15.1/clang_64 make mac-release
```

The resulting Mac OS application can be found `build/bin/feather.app` and will **not** have Tor embedded.
