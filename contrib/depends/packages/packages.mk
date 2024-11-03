packages := boost openssl unbound qrencode libsodium polyseed hidapi abseil protobuf libusb zlib libgpg-error libgcrypt expat libzip zxing-cpp
native_packages := native_qt native_abseil native_protobuf

linux_packages := eudev libfuse libsquashfuse zstd appimage_runtime wayland libffi
linux_native_packages := native_expat native_libffi native_wayland

x86_64_linux_packages := flatstart

darwin_packages :=
darwin_native_packages = darwin_sdk

mingw32_packages =
mingw32_native_packages =

qt_linux_packages := libxcb xcb_proto libXau xorgproto libxkbcommon libxcb_util libxcb_util_render libxcb_util_keysyms libxcb_util_image libxcb_util_cursor libxcb_util_wm freetype fontconfig dbus qt
qt_darwin_packages := qt
qt_mingw32_packages := qt

tor_linux_packages := libevent tor_linux
tor_darwin_packages := tor_darwin
tor_mingw32_packages := tor_mingw32
