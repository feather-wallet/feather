package=qt
$(package)_version=6.5.1
$(package)_download_path=https://download.qt.io/official_releases/qt/6.5/$($(package)_version)/submodules
$(package)_suffix=everywhere-src-$($(package)_version).tar.xz
$(package)_file_name=qtbase-$($(package)_suffix)
$(package)_sha256_hash=db56fa1f4303a1189fe33418d25d1924931c7aef237f89eea9de58e858eebfed
$(package)_darwin_dependencies=native_cctools native_qt openssl
$(package)_mingw32_dependencies=openssl native_qt
$(package)_linux_dependencies=openssl native_qt freetype fontconfig libxcb libxkbcommon libxcb_util libxcb_util_render libxcb_util_keysyms libxcb_util_image libxcb_util_wm libxcb_util_cursor dbus
$(package)_qt_libs=corelib network widgets gui plugins testlib
$(package)_linguist_tools = lrelease lupdate lconvert
$(package)_patches  = aarch64Toolchain.cmake
$(package)_patches += arm64-apple-toolchain.cmake
$(package)_patches += dont_hardcode_pwd.patch
$(package)_patches += fast_fixed_dtoa_no_optimize.patch
$(package)_patches += gnueabihfToolchain.cmake
$(package)_patches += guix_cross_lib_path.patch
$(package)_patches += MacToolchain.cmake
$(package)_patches += no_pthread_cond_clockwait.patch
$(package)_patches += no-renameat2.patch
$(package)_patches += no-statx.patch
$(package)_patches += no_wraprt_on_apple.patch
$(package)_patches += qtbase-moc-ignore-gcc-macro.patch
$(package)_patches += qtmultimedia-fixes.patch
$(package)_patches += rcc_hardcode_timestamp.patch
$(package)_patches += riscvToolchain.cmake
$(package)_patches += root_CMakeLists.txt
$(package)_patches += v4l2.patch
$(package)_patches += windows_func_fix.patch
$(package)_patches += WindowsToolchain.cmake
$(package)_patches += revert_f99ee441.patch
$(package)_patches += CVE-2023-34410-qtbase-6.5.diff
$(package)_patches += CVE-2023-37369-qtbase-6.5.diff
$(package)_patches += xcb-util-image-fix.patch
$(package)_patches += libxau-fix.patch
#$(package)_patches += fix-static-fontconfig-static-linking.patch

$(package)_qttools_file_name=qttools-$($(package)_suffix)
$(package)_qttools_sha256_hash=5744df9e84b2a86f7f932ffc00341c7d7209e741fd1c0679a32b855fcceb2329

$(package)_qtsvg_file_name=qtsvg-$($(package)_suffix)
$(package)_qtsvg_sha256_hash=d58d29491d44f0f59b684686a9898fec0e6c4fb7c09d9393b4e9c211fe9608ef

$(package)_qtwebsockets_file_name=qtwebsockets-$($(package)_suffix)
$(package)_qtwebsockets_sha256_hash=6b8f66b250a675117aae35b48dbfc589619be2810a759ad1712a9cd20561da19

$(package)_qtmultimedia_file_name=qtmultimedia-$($(package)_suffix)
$(package)_qtmultimedia_sha256_hash=0b1fc560e1c8cdda1ddb13db832c3b595f7e4079118d4847d8de18d82464e1cc

$(package)_qtshadertools_file_name=qtshadertools-$($(package)_suffix)
$(package)_qtshadertools_sha256_hash=e5806761835944ef91d5aee0679e0c8231bf7a981e064480e65c751ebdf65052

$(package)_extra_sources += $($(package)_qttools_file_name)
$(package)_extra_sources += $($(package)_qtsvg_file_name)
$(package)_extra_sources += $($(package)_qtwebsockets_file_name)
$(package)_extra_sources += $($(package)_qtmultimedia_file_name)
$(package)_extra_sources += $($(package)_qtshadertools_file_name)

define $(package)_set_vars
$(package)_config_opts_release = -release
$(package)_config_opts_debug = -debug
$(package)_config_opts_debug += -optimized-tools
$(package)_config_opts += -libexecdir $(build_prefix)/bin
$(package)_config_opts += -confirm-license
$(package)_config_opts += -no-cups
$(package)_config_opts += -no-egl
$(package)_config_opts += -no-eglfs
$(package)_config_opts += -no-evdev
$(package)_config_opts += -no-gif
$(package)_config_opts += -no-glib
$(package)_config_opts += -no-icu
$(package)_config_opts += -no-ico
$(package)_config_opts += -no-kms
$(package)_config_opts += -no-linuxfb
#$(package)_config_opts += -no-libjpeg # Needed
#$(package)_config_opts += -no-libproxy # Needed
$(package)_config_opts += -no-libudev
$(package)_config_opts += -no-mtdev
$(package)_config_opts += -openssl-linked
$(package)_config_opts += -no-openvg
$(package)_config_opts += -no-reduce-relocations
$(package)_config_opts += -no-schannel
$(package)_config_opts += -no-sctp
$(package)_config_opts += -no-securetransport
$(package)_config_opts += -no-system-proxies
$(package)_config_opts += -no-use-gold-linker
$(package)_config_opts += -no-zstd
$(package)_config_opts += -nomake examples
$(package)_config_opts += -nomake tests
$(package)_config_opts += -opensource
$(package)_config_opts += -pkg-config
$(package)_config_opts += -prefix $(host_prefix)
$(package)_config_opts += -qt-libpng
$(package)_config_opts += -qt-pcre
$(package)_config_opts += -qt-harfbuzz
$(package)_config_opts += -qt-zlib
$(package)_config_opts += -static
$(package)_config_opts += -no-feature-colordialog
#$(package)_config_opts += -no-feature-concurrent # Needed
$(package)_config_opts += -no-feature-dial
$(package)_config_opts += -no-feature-fontcombobox
#$(package)_config_opts += -no-feature-http # Needed
$(package)_config_opts += -no-feature-image_heuristic_mask
$(package)_config_opts += -no-feature-keysequenceedit
$(package)_config_opts += -no-feature-lcdnumber
$(package)_config_opts += -no-feature-networkdiskcache
#$(package)_config_opts += -no-feature-networkproxy # Needed
$(package)_config_opts += -no-feature-pdf
$(package)_config_opts += -no-feature-printdialog
$(package)_config_opts += -no-feature-printer
$(package)_config_opts += -no-feature-printpreviewdialog
$(package)_config_opts += -no-feature-printpreviewwidget
$(package)_config_opts += -no-feature-printsupport
$(package)_config_opts += -no-feature-sessionmanager
#$(package)_config_opts += -no-feature-socks5 # Needed
$(package)_config_opts += -no-feature-spatialaudio
$(package)_config_opts += -no-feature-sql
$(package)_config_opts += -no-feature-syntaxhighlighter
#$(package)_config_opts += -no-feature-textbrowser # Needed
$(package)_config_opts += -no-feature-textmarkdownwriter
$(package)_config_opts += -no-feature-textodfwriter
$(package)_config_opts += -no-feature-topleveldomain
#$(package)_config_opts += -no-feature-udpsocket # Neede
$(package)_config_opts += -no-feature-undocommand
$(package)_config_opts += -no-feature-undogroup
$(package)_config_opts += -no-feature-undostack
$(package)_config_opts += -no-feature-undoview
$(package)_config_opts += -no-feature-vnc
#$(package)_config_opts += -no-feature-wizard # Needed

$(package)_config_opts_darwin = -no-dbus
$(package)_config_opts_darwin += -no-pch
$(package)_config_opts_darwin += -no-freetype
$(package)_config_opts_darwin += QMAKE_MACOSX_DEPLOYMENT_TARGET=$(OSX_MIN_VERSION)

ifneq ($(build_os),darwin)
$(package)_config_opts_darwin += -xplatform macx-clang
$(package)_config_opts_darwin += -device-option MAC_SDK_PATH=$(host_prefix)/native/SDK
$(package)_config_opts_darwin += -device-option MAC_SDK_VERSION=$(OSX_SDK_VERSION)
$(package)_config_opts_darwin += -device-option CROSS_COMPILE="$(host)-"
$(package)_config_opts_darwin += -device-option MAC_TARGET=$(host)
$(package)_config_opts_darwin += -device-option XCODE_VERSION=$(XCODE_VERSION)
$(package)_config_opts_darwin += -qt-host-path $(build_prefix)/qt-host
endif

$(package)_config_opts_darwin += -no-feature-ffmpeg
$(package)_config_opts_aarch64_darwin += -- -DCMAKE_TOOLCHAIN_FILE=arm64-apple-toolchain.cmake -DCMAKE_LIBRARY_PATH=$(HOME)/.guix-profile/lib
$(package)_config_opts_x86_64_darwin += -- -DCMAKE_TOOLCHAIN_FILE=MacToolchain.cmake -DCMAKE_LIBRARY_PATH=$(HOME)/.guix-profile/lib

$(package)_config_opts_linux = -xcb
$(package)_config_opts_linux += -no-xcb-xlib
$(package)_config_opts_linux += -no-feature-xlib
#$(package)_config_opts_linux += -feature-ffmpeg
#$(package)_config_opts_linux += -feature-pulseaudio

# https://bugreports.qt.io/browse/QTBUG-99957
$(package)_config_opts_linux += -no-pch

$(package)_config_opts_linux += -system-freetype
$(package)_config_opts_linux += -fontconfig
$(package)_config_opts_linux += -no-opengl
$(package)_config_opts_linux += -no-feature-vulkan
$(package)_config_opts_linux += -no-feature-backtrace
$(package)_config_opts_linux += -dbus-linked
ifneq ($(LTO),)
$(package)_config_opts_linux += -ltcg
endif
$(package)_config_opts_linux += -platform linux-g++ -xplatform linux-g++
ifneq (,$(findstring -stdlib=libc++,$($(1)_cxx)))
$(package)_config_opts_x86_64_linux = -xplatform linux-clang-libc++
endif

$(package)_config_opts_arm_linux += -qt-host-path $(build_prefix)/qt-host
$(package)_config_opts_arm_linux += -- -DCMAKE_TOOLCHAIN_FILE=gnueabihfToolchain.cmake -DCMAKE_LIBRARY_PATH=$(HOME)/.guix-profile/lib

$(package)_config_opts_aarch64_linux += -qt-host-path $(build_prefix)/qt-host
$(package)_config_opts_aarch64_linux += -- -DCMAKE_TOOLCHAIN_FILE=aarch64Toolchain.cmake -DCMAKE_LIBRARY_PATH=$(HOME)/.guix-profile/lib

$(package)_config_opts_riscv64_linux += -qt-host-path $(build_prefix)/qt-host
$(package)_config_opts_riscv64_linux += -- -DCMAKE_TOOLCHAIN_FILE=riscvToolchain.cmake -DCMAKE_LIBRARY_PATH=$(HOME)/.guix-profile/lib

$(package)_config_opts_mingw32 += -no-opengl
$(package)_config_opts_mingw32 += -no-dbus
$(package)_config_opts_mingw32 += -no-freetype
$(package)_config_opts_mingw32 += -xplatform win32-g++
$(package)_config_opts_mingw32 += -device-option CROSS_COMPILE="$(host)-"
$(package)_config_opts_mingw32 += -pch
$(package)_config_opts_mingw32 += -qt-host-path $(build_prefix)/qt-host
$(package)_config_opts_mingw32 += -wmf
$(package)_config_opts_mingw32 += -- -DCMAKE_TOOLCHAIN_FILE=WindowsToolchain.cmake -DCMAKE_LIBRARY_PATH=$(HOME)/.guix-profile/lib

$(package)_openssl_flags_$(host_os)="-lssl -lcrypto -lpthread -ldl"
$(package)_openssl_flags_mingw32="-lssl -lcrypto -lws2_32"
endef

define $(package)_fetch_cmds
$(call fetch_file,$(package),$($(package)_download_path),$($(package)_download_file),$($(package)_file_name),$($(package)_sha256_hash)) && \
$(call fetch_file,$(package),$($(package)_download_path),$($(package)_qttools_file_name),$($(package)_qttools_file_name),$($(package)_qttools_sha256_hash)) && \
$(call fetch_file,$(package),$($(package)_download_path),$($(package)_qtsvg_file_name),$($(package)_qtsvg_file_name),$($(package)_qtsvg_sha256_hash)) && \
$(call fetch_file,$(package),$($(package)_download_path),$($(package)_qtwebsockets_file_name),$($(package)_qtwebsockets_file_name),$($(package)_qtwebsockets_sha256_hash)) && \
$(call fetch_file,$(package),$($(package)_download_path),$($(package)_qtmultimedia_file_name),$($(package)_qtmultimedia_file_name),$($(package)_qtmultimedia_sha256_hash)) && \
$(call fetch_file,$(package),$($(package)_download_path),$($(package)_qtshadertools_file_name),$($(package)_qtshadertools_file_name),$($(package)_qtshadertools_sha256_hash))
endef

define $(package)_extract_cmds
  mkdir -p $($(package)_extract_dir) && \
  echo "$($(package)_sha256_hash)  $($(package)_source)" > $($(package)_extract_dir)/.$($(package)_file_name).hash && \
  echo "$($(package)_qttools_sha256_hash)  $($(package)_source_dir)/$($(package)_qttools_file_name)" >> $($(package)_extract_dir)/.$($(package)_file_name).hash && \
  echo "$($(package)_qtsvg_sha256_hash)  $($(package)_source_dir)/$($(package)_qtsvg_file_name)" >> $($(package)_extract_dir)/.$($(package)_file_name).hash && \
  echo "$($(package)_qtwebsockets_sha256_hash)  $($(package)_source_dir)/$($(package)_qtwebsockets_file_name)" >> $($(package)_extract_dir)/.$($(package)_file_name).hash && \
  echo "$($(package)_qtmultimedia_sha256_hash)  $($(package)_source_dir)/$($(package)_qtmultimedia_file_name)" >> $($(package)_extract_dir)/.$($(package)_file_name).hash && \
  echo "$($(package)_qtshadertools_sha256_hash)  $($(package)_source_dir)/$($(package)_qtshadertools_file_name)" >> $($(package)_extract_dir)/.$($(package)_file_name).hash && \
  $(build_SHA256SUM) -c $($(package)_extract_dir)/.$($(package)_file_name).hash && \
  mkdir qtbase && \
  $(build_TAR) --no-same-owner --strip-components=1 -xf $($(package)_source) -C qtbase && \
  mkdir qttools && \
  $(build_TAR) --no-same-owner --strip-components=1 -xf $($(package)_source_dir)/$($(package)_qttools_file_name) -C qttools && \
  mkdir qtsvg && \
  $(build_TAR) --no-same-owner --strip-components=1 -xf $($(package)_source_dir)/$($(package)_qtsvg_file_name) -C qtsvg && \
  mkdir qtwebsockets && \
  $(build_TAR) --no-same-owner --strip-components=1 -xf $($(package)_source_dir)/$($(package)_qtwebsockets_file_name) -C qtwebsockets && \
  mkdir qtmultimedia && \
  $(build_TAR) --no-same-owner --strip-components=1 -xf $($(package)_source_dir)/$($(package)_qtmultimedia_file_name) -C qtmultimedia && \
  mkdir qtshadertools && \
  $(build_TAR) --no-same-owner --strip-components=1 -xf $($(package)_source_dir)/$($(package)_qtshadertools_file_name) -C qtshadertools
endef

define $(package)_preprocess_cmds
  cp $($(package)_patch_dir)/root_CMakeLists.txt CMakeLists.txt && \
  patch -p1 -i $($(package)_patch_dir)/dont_hardcode_pwd.patch && \
  patch -p1 -i $($(package)_patch_dir)/qtbase-moc-ignore-gcc-macro.patch && \
  patch -p1 -i $($(package)_patch_dir)/rcc_hardcode_timestamp.patch && \
  patch -p1 -i $($(package)_patch_dir)/fast_fixed_dtoa_no_optimize.patch && \
  patch -p1 -i $($(package)_patch_dir)/guix_cross_lib_path.patch && \
  patch -p1 -i $($(package)_patch_dir)/no-statx.patch && \
  patch -p1 -i $($(package)_patch_dir)/no-renameat2.patch && \
  patch -p1 -i $($(package)_patch_dir)/no_pthread_cond_clockwait.patch && \
  patch -p1 -i $($(package)_patch_dir)/windows_func_fix.patch && \
  patch -p1 -i $($(package)_patch_dir)/no_wraprt_on_apple.patch && \
  mv $($(package)_patch_dir)/WindowsToolchain.cmake . && \
  mv $($(package)_patch_dir)/MacToolchain.cmake . && \
  sed -i -e 's|@cmake_c_flags@|$(darwin_CC_)|' \
      -e 's|@cmake_cxx_flags@|$(darwin_CXX_)|' \
      MacToolchain.cmake && \
  mv $($(package)_patch_dir)/aarch64Toolchain.cmake . && \
  mv $($(package)_patch_dir)/arm64-apple-toolchain.cmake . && \
  sed -i -e 's|@cmake_c_flags@|$(darwin_CC_)|' \
      -e 's|@cmake_cxx_flags@|$(darwin_CXX_)|' \
      arm64-apple-toolchain.cmake && \
  mv $($(package)_patch_dir)/gnueabihfToolchain.cmake . && \
  mv $($(package)_patch_dir)/riscvToolchain.cmake . && \
  cd qtbase && \
  patch -p1 -i $($(package)_patch_dir)/revert_f99ee441.patch && \
  patch -p1 -i $($(package)_patch_dir)/CVE-2023-34410-qtbase-6.5.diff && \
  patch -p1 -i $($(package)_patch_dir)/xcb-util-image-fix.patch && \
  patch -p1 -i $($(package)_patch_dir)/libxau-fix.patch && \
  patch -pi -i $($(package)_patch_dir)/CVE-2023-37369-qtbase-6.5.diff && \
  cd ../qtmultimedia && \
  patch -p1 -i $($(package)_patch_dir)/qtmultimedia-fixes.patch && \
  patch -p1 -i $($(package)_patch_dir)/v4l2.patch
endef

# TODO: this is a mess, but i'm tired of rebuilding Qt so it is what it is
# TODO: find a better way to make WMF libraries available to Qt without polluting the environment
ifeq ($(host_os),darwin)
define $(package)_config_cmds
  export OPENSSL_LIBS=${$(package)_openssl_flags_$(host_os)} \
  export PKG_CONFIG_SYSROOT_DIR=/ && \
  export PKG_CONFIG_LIBDIR=$(host_prefix)/lib/pkgconfig && \
  export QT_MAC_SDK_NO_VERSION_CHECK=1 && \
  cd qtbase && \
  env -u C_INCLUDE_PATH -u CPLUS_INCLUDE_PATH -u OBJC_INCLUDE_PATH -u OBJCPLUS_INCLUDE_PATH -u CPATH -u LIBRARY_PATH ./configure -top-level $($(package)_config_opts) -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON
endef
else ifeq ($(host_os),mingw32)
define $(package)_config_cmds
  cp $(WMF_LIBS)/lib/libstrmiids.a \
     $(WMF_LIBS)/lib/libamstrmid.a \
     $(WMF_LIBS)/lib/libdmoguids.a \
     $(WMF_LIBS)/lib/libuuid.a \
     $(WMF_LIBS)/lib/libmsdmo.a \
     $(WMF_LIBS)/lib/libole32.a \
     $(WMF_LIBS)/lib/liboleaut32.a \
     $(WMF_LIBS)/lib/libmf.a \
     $(WMF_LIBS)/lib/libmfuuid.a \
     $(WMF_LIBS)/lib/libmfplat.a \
     $(WMF_LIBS)/lib/libmfcore.a \
     $(WMF_LIBS)/lib/libpropsys.a \
     /feather/contrib/depends/x86_64-w64-mingw32/lib/ && \
   export OPENSSL_LIBS=${$(package)_openssl_flags_$(host_os)} \
   export PKG_CONFIG_SYSROOT_DIR=/ && \
   export PKG_CONFIG_LIBDIR=$(host_prefix)/lib/pkgconfig && \
   export QT_MAC_SDK_NO_VERSION_CHECK=1 && \
   export V=1 && \
   export VERBOSE=1 && \
   cd qtbase && \
  ./configure -top-level $($(package)_config_opts) -DCMAKE_LIBRARY_PATH=$(HOME)/.guix-profile/lib -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON
endef
else
define $(package)_config_cmds
  export OPENSSL_LIBS=${$(package)_openssl_flags_$(host_os)} \
  export PKG_CONFIG_SYSROOT_DIR=/ && \
  export PKG_CONFIG_LIBDIR=$(host_prefix)/lib/pkgconfig && \
  export QT_MAC_SDK_NO_VERSION_CHECK=1 && \
  cd qtbase && \
  ./configure -top-level $($(package)_config_opts)
endef
endif

ifeq ($(host_os),darwin)
define $(package)_build_cmds
  export LD_LIBRARY_PATH="${build_prefix}/lib/:$(QT_LIBS_LIBS)" && \
  env -u C_INCLUDE_PATH -u CPLUS_INCLUDE_PATH -u OBJC_INCLUDE_PATH -u OBJCPLUS_INCLUDE_PATH -u CPATH -u LIBRARY_PATH cmake --build . --parallel
endef
else
define $(package)_build_cmds
  export LD_LIBRARY_PATH="${build_prefix}/lib/:$(QT_LIBS_LIBS)" && \
  cmake --build . --parallel
endef
endif

define $(package)_stage_cmds
  DESTDIR=$($(package)_staging_dir) cmake --install .
endef
