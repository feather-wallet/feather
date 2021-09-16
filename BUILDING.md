## Building with Docker

Builds with Docker are done in 3 steps:

1. Cloning this repository (+submodules)
2. Creating a base image containing the build environment
3. Building Feather using the base image

### Linux x86-64 (reproducible)

The instructions in this section are for 64-bit AMD/Intel processors. For ARM64 platforms see the next section.

Binaries produced in this section are reproducible and their digests should match those of release binaries.

#### 1. Clone

Replace `master` with the desired version tag (e.g. `beta-9`) to build the release binary.

```bash
git clone https://git.featherwallet.org/feather/feather.git
cd feather
git checkout master
git submodule update --init --recursive
```

#### 2. Base image

```bash
docker build -t feather:linux -f Dockerfile.linux --build-arg THREADS=8 .
```

Building the base image takes a while. You only need to build the base image once per release.

#### 3. Build

##### Standalone static binary

If you're re-running a build make sure to `rm -rf build/` first.

```bash
docker run --rm -it -v $PWD:/feather -w /feather feather:linux sh -c 'WITH_SCANNER=Off make release-static -j8'
```

The resulting binary can be found in `./build/bin/`.

##### AppImage

```bash
rm -rf build
docker run --rm -it -v $PWD:/feather -w /feather feather:linux sh -c 'make release-static -j8'
docker run --rm -it -v $PWD:/feather -w /feather/build feather:linux ../contrib/build-appimage.sh
```

The resulting AppImage will be located in `./build`.

### Linux ARM64 (reproducible)

This section describes how to build Feather for ARM64 based platforms (including Raspberry Pi 3 and later, running 64-bit Rasbian).

Binaries produced in this section are not yet reproducible.

#### 1. Clone

Replace `master` with the desired version tag (e.g. `beta-9`) to build the release binary.

```bash
git clone https://git.featherwallet.org/feather/feather.git
cd feather
git checkout master
git submodule update --init --recursive
```

#### 2. Base image

```bash
docker build --tag feather:linux-arm64 --platform linux/arm64 -f Dockerfile.linux --build-arg THREADS=8 .
```

Building the base image takes a while (especially when emulated on x86-64). You only need to build the base image once per release.

#### 3. Build

##### Standalone static binary

```bash
docker run --platform linux/arm64/v8 --rm -it -v $PWD:/feather -w /feather feather:linux-arm64 sh -c 'WITH_SCANNER=Off make release-static-linux-arm64 -j8'
```

Note: If you intend to run Feather on a Raspberry Pi or any device without AES hardware acceleration, use the following command instead:

```bash
docker run --platform linux/arm64/v8 --rm -it -v $PWD:/feather -w /feather feather:linux-arm64 sh -c 'WITH_SCANNER=Off make release-static-linux-arm64-rpi -j8'
```

The resulting binary can be found in `./build/bin/`.

##### AppImage

##### ARMv8-a (with AES hardware acceleration)

```bash
rm -rf build
docker run --rm -it -v $PWD:/feather --platform linux/arm64/v8 -w /feather feather:linux-arm64 sh -c 'make release-static-linux-arm64 -j8'
docker run --rm -it -v $PWD:/feather --platform linux/arm64/v8 -w /feather/build feather:linux-arm64 ../contrib/build-appimage-arm64.sh
```

##### Raspberry Pi

```bash
rm -rf build
docker run --rm -it -v $PWD:/feather --platform linux/arm64/v8 -w /feather feather:linux-arm64 sh -c 'make release-static-linux-arm64-rpi -j8'
docker run --rm -it -v $PWD:/feather --platform linux/arm64/v8 -w /feather/build feather:linux-arm64 ../contrib/build-appimage-arm64.sh
```

The resulting AppImage will be located in `./build`.

### Windows (reproducible)

#### 1. Clone

```bash
git clone --branch master --recursive https://git.featherwallet.org/feather/feather.git
cd feather
```

Replace `master` with the desired version tag (e.g. `beta-9`) to build the release binary.

#### 2. Base image


```bash
docker build -f Dockerfile.windows --tag feather:win --build-arg THREADS=4 .
```

Building the base image takes a while. You only need to build the base image once.

#### 3. Build

```bash
docker run --rm -it -v $PWD:/feather -w /feather feather:win sh -c 'make depends root=/depends target=x86_64-w64-mingw32 tag=win-x64 -j4'
```

If you're re-running a build make sure to `rm -rf build/` first.

The resulting binary can be found in `./build/x86_64-w64-mingw32/release/bin/`.

---

## Building on macOS

For macOS it's easiest to leverage [brew](https://brew.sh) to install the required dependencies. 

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

The resulting macOS application can be found `build/bin/feather.app` and will **not** have Tor embedded.
