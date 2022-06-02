native_packages := native_cmake
packages := boost openssl libiconv ldns unbound qrencode zbar sodium polyseed hidapi protobuf libusb zlib libgpg-error libgcrypt ncurses readline expat

hardware_packages := hidapi protobuf libusb
hardware_native_packages := native_protobuf

linux_packages := eudev libzip
linux_native_packages = $(hardware_native_packages) native_patchelf
# native_linuxdeployqt native_squashfs-tools native_qmake

qt_linux_packages:=qt expat libxcb xcb_proto libXau xproto freetype fontconfig libxkbcommon libxcb_util libxcb_util_render libxcb_util_keysyms libxcb_util_image libxcb_util_wm
qt_darwin_packages=qt
qt_mingw32_packages=qt

# apt install python3-setuptools
darwin_packages := libzip
darwin_native_packages = $(hardware_native_packages) native_ds_store native_mac_alias

tor_linux_packages := libevent tor
tor_darwin_packages := tor-macos

mingw32_packages = icu4c sodium $(hardware_packages) tor-win libzip
mingw32_native_packages = $(hardware_native_packages)

ifneq ($(build_os),darwin)
darwin_native_packages += native_cctools native_libtapi

ifeq ($(strip $(FORCE_USE_SYSTEM_CLANG)),)
darwin_native_packages+= native_clang
endif

endif
