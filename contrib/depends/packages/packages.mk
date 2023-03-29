packages := boost openssl libiconv unbound qrencode zbar sodium polyseed hidapi protobuf libusb zlib libgpg-error libgcrypt expat libzip
native_packages := native_libxcb native_xcb_proto native_libXau native_xproto native_libxkbcommon native_qt native_protobuf

linux_packages := eudev liblzma libarchive libfuse libsquashfuse libappimage appimage_runtime
linux_native_packages =

darwin_packages :=
darwin_native_packages = darwin_sdk native_cctools native_libtapi native_clang

mingw32_packages = icu4c
mingw32_native_packages =

qt_linux_packages := libxcb xcb_proto libXau xproto libxkbcommon libxcb_util libxcb_util_render libxcb_util_keysyms libxcb_util_image libxcb_util_cursor libxcb_util_wm freetype fontconfig qt
qt_darwin_packages := qt
qt_mingw32_packages := qt

tor_linux_packages := libevent tor_linux
tor_darwin_packages := tor_darwin
tor_mingw32_packages := tor_mingw32