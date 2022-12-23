native_packages := native_cmake
packages := boost openssl libiconv ldns unbound qrencode zbar sodium polyseed hidapi protobuf libusb zlib libgpg-error libgcrypt expat

hardware_packages := hidapi protobuf libusb
hardware_native_packages := native_protobuf

linux_packages := eudev libzip liblzma libarchive libfuse libsquashfuse libappimage appimage_runtime
linux_native_packages = $(hardware_native_packages) native_patchelf

linux_native_packages += native_qt native_expat native_libxcb native_xcb_proto native_libXau native_xproto native_freetype native_fontconfig native_libxkbcommon native_libxcb_util native_libxcb_util_render native_libxcb_util_keysyms native_libxcb_util_image native_libxcb_util_wm

qt_linux_packages:=native_expat libxcb xcb_proto libXau xproto native_freetype native_fontconfig libxkbcommon libxcb_util libxcb_util_render libxcb_util_keysyms libxcb_util_image libxcb_util_wm qt
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