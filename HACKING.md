# Documentation for developers

Feather is developed primarily on Linux, but can also be built on macOS. Development on Windows is not currently supported.

We support development on rolling release distributions and the latest version of Ubuntu. Building on older stable distributions is not guaranteed to work.

## Setting up a development environment

### Dependencies

#### Arch Linux

```bash
pacman -S git cmake base-devel ccache unbound boost qrencode qt6-base qt6-svg qt6-websockets qt6-wayland qt6-multimedia libzip hidapi protobuf zxing-cpp
```

#### Ubuntu 24.04

```bash
apt update
apt install git cmake build-essential ccache libssl-dev libunbound-dev libboost-all-dev \
            libqrencode-dev qt6-base-dev qt6-svg-dev qt6-websockets-dev qt6-multimedia-dev \
            qt6-wayland-dev libzip-dev libsodium-dev libgcrypt20-dev libx11-xcb-dev \
            libprotobuf-dev libhidapi-dev libzxing-dev libusb-1.0-0-dev
```

#### Rhel 9 / Alma / Rocky

```bash
sudo dnf install epel-release -y
sudo dnf groupinstall "Development Tools" -y
sudo dnf install unbound-devel boost-devel qrencode-devel zxing-cpp-devel qt6-qtbase-devel qt6-qtsvg-devel qt6-qtmultimedia-devel qt6-qtwayland-devel libsodium-devel
```

#### Void Linux

```bash
xbps-install -S base-devel cmake boost-devel openssl-devel unbound-devel libsodium-devel zlib-devel qt6-base-devel \
                qt6-svg-devel qt6-multimedia-devel qt6-wayland-devel libgcrypt-devel libzip-devel \
                hidapi-devel protobuf protobuf-devel qrencode-devel zxing-cpp-devel
```

#### macOS

For macOS it's easiest to leverage [brew](https://brew.sh) to install the required dependencies.

```bash
brew install qt libsodium libzip qrencode unbound cmake boost hidapi openssl expat libunwind-headers protobuf pkgconfig
```

Build [zxing-cpp](https://github.com/zxing-cpp/zxing-cpp) from source or compile Feather with `-DWITH_SCANNER=Off`.

#### Arch Linux

```bash
pacman -S tor
systemctl enable --now tor
```

#### Ubuntu Debian

```bash
apt update && apt install tor
systemctl enable --now tor
```

#### Void Linux

```bash
xbps-install tor
ln -s /etc/sv/tor /var/service/.
sv start tor
```

#### macOS

```bash
brew install tor
brew services restart tor
```

### Clone Feather

```bash
git clone http://github.com/feather-wallet/feather.git
cd feather
git submodule update --init --recursive
```

### Jetbrains Clion (IDE)

We recommend using Jetbrains Clion for Feather development. It integrates nicely with CMake and comes with a built-in
debugger. 

To pass CMake flags to CLion, go to `File -> Settings -> Build -> CMake`, set Build Type to `Debug` and set your
preferred CMake options.  More CMake options are documented below.

Run CMake (`View -> Tool Windows -> CMake`). Click on the 🔃 (`Reload CMake Project`) button.

Go to `Run -> Edit configurations` and make sure the `feather` target is selected. 
You can add any environment variables and program arguments here:

- For more verbose logging add `MONERO_LOG_LEVEL=1` to environment variables.
- To start Feather in stagenet mode, add `--stagenet` to program arguments. 

After the target is configured, `Run -> Run 'feather'` or press Shift + F10 to build Feather.

### Building without IDE

To build Feather without an IDE:

```bash
mkdir build
cd build
cmake ..
cmake --build . -j $(nproc)
```

On platforms without `execinfo.h` use `cmake -DSTACK_TRACE:BOOL=OFF ..` instead of `cmake ..`

### CMake

There are some CMake options that you may pass to control how Feather is built:

- `-DCHECK_UPDATES=ON` - enable checking for updates, only for standalone binaries
- `-DDONATE_BEG=OFF` - disable the dreaded donate requests
- `-DUSE_DEVICE_TREZOR=OFF` - disable Trezor hardware wallet support
- `-DWITH_SCANNER=ON` - enable the webcam QR code scanner
