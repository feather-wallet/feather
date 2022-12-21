native_packages := native_cmake
packages := boost openssl libiconv ldns unbound qrencode zbar sodium polyseed hidapi protobuf libusb zlib libgpg-error libgcrypt expat

hardware_packages := hidapi protobuf libusb
hardware_native_packages := native_protobuf

linux_packages := eudev libzip liblzma libarchive libfuse libsquashfuse libappimage
linux_native_packages = $(hardware_native_packages) native_patchelf

qt_linux_packages:=qt native_expat native_libxcb native_xcb_proto native_libXau native_xproto native_freetype native_fontconfig native_libxkbcommon native_libxcb_util native_libxcb_util_render native_libxcb_util_keysyms native_libxcb_util_image native_libxcb_util_wm appimage_runtime
qt_darwin_packages=native_qt qt
qt_mingw32_packages=native_qt

darwin_packages := libzip
darwin_native_packages = $(hardware_native_packages) native_ds_store native_mac_alias native_expat native_libxcb native_xcb_proto native_libXau native_xproto native_freetype native_fontconfig native_libxkbcommon native_libxcb_util native_libxcb_util_render native_libxcb_util_keysyms native_libxcb_util_image native_libxcb_util_wm

tor_linux_packages := libevent tor_linux
tor_darwin_packages := tor_darwin

mingw32_packages = icu4c sodium $(hardware_packages) tor_mingw32 libzip qt
mingw32_native_packages = $(hardware_native_packages) native_expat native_libxcb native_xcb_proto native_libXau native_xproto native_freetype native_fontconfig native_libxkbcommon native_libxcb_util native_libxcb_util_render native_libxcb_util_keysyms native_libxcb_util_image native_libxcb_util_wm

ifneq ($(build_os),darwin)
darwin_native_packages += native_cctools native_libtapi

ifeq ($(strip $(FORCE_USE_SYSTEM_CLANG)),)
darwin_native_packages+= native_clang
endif

endif