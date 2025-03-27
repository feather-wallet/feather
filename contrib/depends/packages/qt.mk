package=qt
$(package)_version=6.8.3
$(package)_download_path=https://download.qt.io/official_releases/qt/6.8/$($(package)_version)/submodules
$(package)_suffix=everywhere-src-$($(package)_version).tar.xz
$(package)_file_name=qtbase-$($(package)_suffix)
$(package)_sha256_hash=56001b905601bb9023d399f3ba780d7fa940f3e4861e496a7c490331f49e0b80
$(package)_darwin_dependencies=openssl native_qt
$(package)_mingw32_dependencies=openssl native_qt
$(package)_linux_dependencies=openssl native_qt freetype fontconfig libxcb libxkbcommon libxcb_util libxcb_util_render libxcb_util_keysyms libxcb_util_image libxcb_util_wm libxcb_util_cursor dbus wayland native_wayland
$(package)_patches += fast_fixed_dtoa_no_optimize.patch
$(package)_patches += guix_cross_lib_path.patch
$(package)_patches += qtbase-moc-ignore-gcc-macro.patch
$(package)_patches += qtmultimedia-fixes.patch
$(package)_patches += rcc_hardcode_timestamp.patch
$(package)_patches += root_CMakeLists.txt
$(package)_patches += v4l2.patch
$(package)_patches += windows_func_fix.patch
$(package)_patches += xcb-util-image-fix.patch
$(package)_patches += libxau-fix.patch
$(package)_patches += toolchain.cmake
$(package)_patches += revert-macOS-Silence-warning-about-supporting-secure.patch
$(package)_patches += no-resonance-audio.patch
$(package)_patches += fix_static_qt_darwin_camera_permissions.patch
$(package)_patches += macos-available-qtbase.patch
$(package)_patches += macos-available-qtmultimedia.patch
$(package)_patches += qtwayland-tabletevent-fix.patch
#$(package)_patches += fix-static-fontconfig-static-linking.patch

$(package)_qttools_file_name=qttools-$($(package)_suffix)
$(package)_qttools_sha256_hash=02a4e219248b94f1333df843d25763f35251c1074cdc4fb5bda67d340f8c8b3a

$(package)_qtsvg_file_name=qtsvg-$($(package)_suffix)
$(package)_qtsvg_sha256_hash=35eb516460f00f264eb504baa253432384351cf23fb9980a5857190e8deef438

$(package)_qtwebsockets_file_name=qtwebsockets-$($(package)_suffix)
$(package)_qtwebsockets_sha256_hash=fc6310c1b12d6e22a31062727aa6574ed69239d036c8a4247b3b089902709b11

$(package)_qtmultimedia_file_name=qtmultimedia-$($(package)_suffix)
$(package)_qtmultimedia_sha256_hash=32e82307d783172a3b984cc3c47c5e4e8b819cee3cbfc702c7012c47f15f6b01

$(package)_qtshadertools_file_name=qtshadertools-$($(package)_suffix)
$(package)_qtshadertools_sha256_hash=f6ec88bf42deba84d8f6b5d0914636ceed4749ccb51d1945b2f79b322b7ecf47

$(package)_qtwayland_file_name=qtwayland-$($(package)_suffix)
$(package)_qtwayland_sha256_hash=20fe385887d21190165a3180c17dcfc8b9a0e1da4ec76865b6334bdc709994b0

$(package)_extra_sources += $($(package)_qttools_file_name)
$(package)_extra_sources += $($(package)_qtsvg_file_name)
$(package)_extra_sources += $($(package)_qtwebsockets_file_name)
$(package)_extra_sources += $($(package)_qtmultimedia_file_name)
$(package)_extra_sources += $($(package)_qtshadertools_file_name)
$(package)_extra_sources += $($(package)_qtwayland_file_name)

define $(package)_set_vars
$(package)_config_opts += -DQT_HOST_PATH=$(build_prefix)/qt-host
$(package)_config_opts += -DCMAKE_LIBRARY_PATH=/home/user/.guix-profile/lib
$(package)_config_opts += -DBUILD_SHARED_LIBS=OFF
$(package)_config_opts += -DCMAKE_INSTALL_PREFIX=$(host_prefix)
$(package)_config_opts += -DINSTALL_LIBEXECDIR=$(build_prefix)/bin
$(package)_config_opts += -DQT_BUILD_EXAMPLES=FALSE
$(package)_config_opts += -DQT_BUILD_TESTS=FALSE
$(package)_config_opts += -DQT_GENERATE_SBOM=OFF
$(package)_config_opts += -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake
$(package)_config_opts += -DQT_FEATURE_cups=OFF
$(package)_config_opts += -DQT_FEATURE_qmake=OFF
$(package)_config_opts += -DQT_FEATURE_egl=OFF
$(package)_config_opts += -DQT_FEATURE_egl_x11=OFF
$(package)_config_opts += -DQT_FEATURE_xcb_egl_plugin=OFF
$(package)_config_opts += -DQT_FEATURE_xcb_glx_plugin=OFF
$(package)_config_opts += -DQT_FEATURE_eglfs=OFF
$(package)_config_opts += -DQT_FEATURE_evdev=OFF
$(package)_config_opts += -DQT_FEATURE_gif=OFF
$(package)_config_opts += -DQT_FEATURE_glib=OFF
$(package)_config_opts += -DQT_FEATURE_icu=OFF
$(package)_config_opts += -DQT_FEATURE_ico=OFF
$(package)_config_opts += -DQT_FEATURE_kms=OFF
$(package)_config_opts += -DQT_FEATURE_linuxfb=OFF
$(package)_config_opts += -DQT_FEATURE_libudev=OFF
$(package)_config_opts += -DQT_FEATURE_mtdev=OFF
$(package)_config_opts += -DQT_FEATURE_openssl=ON
$(package)_config_opts += -DQT_FEATURE_openssl_linked=ON
$(package)_config_opts += -DQT_FEATURE_openvg=OFF
$(package)_config_opts += -DQT_FEATURE_permissions=ON
$(package)_config_opts += -DQT_FEATURE_reduce_relocations=OFF
$(package)_config_opts += -DQT_FEATURE_schannel=OFF
$(package)_config_opts += -DQT_FEATURE_sctp=OFF
$(package)_config_opts += -DQT_FEATURE_securetransport=OFF
$(package)_config_opts += -DQT_FEATURE_system_proxies=OFF
$(package)_config_opts += -DQT_FEATURE_use_gold_linker_alias=OFF
$(package)_config_opts += -DQT_FEATURE_zstd=OFF
$(package)_config_opts += -DQT_FEATURE_pkg_config=ON
$(package)_config_opts += -DQT_FEATURE_system_png=OFF
$(package)_config_opts += -DQT_FEATURE_system_pcre2=OFF
$(package)_config_opts += -DQT_FEATURE_system_harfbuzz=OFF
$(package)_config_opts += -DQT_FEATURE_system_zlib=OFF
$(package)_config_opts += -DQT_FEATURE_colordialog=OFF
$(package)_config_opts += -DQT_FEATURE_dial=OFF
$(package)_config_opts += -DQT_FEATURE_fontcombobox=OFF
$(package)_config_opts += -DQT_FEATURE_image_heuristic_mask=OFF
$(package)_config_opts += -DQT_FEATURE_keysequenceedit=OFF
$(package)_config_opts += -DQT_FEATURE_lcdnumber=OFF
$(package)_config_opts += -DQT_FEATURE_networkdiskcache=OFF
$(package)_config_opts += -DQT_FEATURE_pdf=OFF
$(package)_config_opts += -DQT_FEATURE_printdialog=OFF
$(package)_config_opts += -DQT_FEATURE_printer=OFF
$(package)_config_opts += -DQT_FEATURE_printpreviewdialog=OFF
$(package)_config_opts += -DQT_FEATURE_printpreviewwidget=OFF
$(package)_config_opts += -DQT_FEATURE_printsupport=OFF
$(package)_config_opts += -DQT_FEATURE_sessionmanager=OFF
$(package)_config_opts += -DQT_FEATURE_spatialaudio=OFF
$(package)_config_opts += -DQT_FEATURE_sql=OFF
$(package)_config_opts += -DQT_FEATURE_syntaxhighlighter=OFF
$(package)_config_opts += -DQT_FEATURE_tabletevent=OFF
$(package)_config_opts += -DQT_FEATURE_textmarkdownwriter=OFF
$(package)_config_opts += -DQT_FEATURE_textodfwriter=OFF
$(package)_config_opts += -DQT_FEATURE_topleveldomain=OFF
$(package)_config_opts += -DQT_FEATURE_undocommand=OFF
$(package)_config_opts += -DQT_FEATURE_undogroup=OFF
$(package)_config_opts += -DQT_FEATURE_undostack=OFF
$(package)_config_opts += -DQT_FEATURE_undoview=OFF
$(package)_config_opts += -DQT_FEATURE_vnc=OFF

$(package)_config_opts_linux += -DQT_QMAKE_TARGET_MKSPEC=linux-g++
$(package)_config_opts_linux += -DQT_FEATURE_xcb=ON
$(package)_config_opts_linux += -DQT_FEATURE_xcb_xlib=OFF
$(package)_config_opts_linux += -DQT_FEATURE_xlib=OFF
$(package)_config_opts_linux += -DQT_FEATURE_freetype=ON
$(package)_config_opts_linux += -DQT_FEATURE_system_freetype=ON
$(package)_config_opts_linux += -DQT_FEATURE_fontconfig=ON
$(package)_config_opts_linux += -DINPUT_opengl=no
$(package)_config_opts_linux += -DQT_FEATURE_opengl=OFF
$(package)_config_opts_linux += -DQT_FEATURE_opengles2=OFF
$(package)_config_opts_linux += -DQT_FEATURE_opengles3=OFF
$(package)_config_opts_linux += -DQT_FEATURE_opengles31=OFF
$(package)_config_opts_linux += -DQT_FEATURE_opengles32=OFF
$(package)_config_opts_linux += -DQT_FEATURE_opengl_desktop=OFF
$(package)_config_opts_linux += -DQT_FEATURE_vulkan=OFF
$(package)_config_opts_linux += -DQT_FEATURE_backtrace=OFF
$(package)_config_opts_linux += -DQT_FEATURE_dbus=ON
$(package)_config_opts_linux += -DQT_FEATURE_dbus_linked=ON
$(package)_config_opts_linux += -DQT_FEATURE_wayland_client=ON
$(package)_config_opts_linux += -DQT_FEATURE_wayland_server=OFF
$(package)_config_opts_linux += -DQT_FEATURE_wayland_drm_egl_server_buffer=OFF
$(package)_config_opts_linux += -DQT_FEATURE_wayland-shm-emulation-server-buffer=OFF
$(package)_config_opts_linux += -DQT_FEATURE_wayland-client-fullscreen-shell-v1=OFF
$(package)_config_opts_linux += -DQT_FEATURE_wayland-client-ivi-shell=OFF
$(package)_config_opts_linux += -DQT_FEATURE_wayland-client-wl-shell=OFF
$(package)_config_opts_linux += -DQT_FEATURE_wayland-client-xdg-shell-v5=OFF
$(package)_config_opts_linux += -DQT_FEATURE_wayland-client-xdg-shell-v6=OFF
$(package)_config_opts_linux += -DBUILD_WITH_PCH=OFF

$(package)_config_opts_mingw32 += -DQT_QMAKE_TARGET_MKSPEC=win32-g++
$(package)_config_opts_mingw32 += -DINPUT_opengl=no
$(package)_config_opts_mingw32 += -DQT_FEATURE_dbus=OFF
$(package)_config_opts_mingw32 += -DQT_FEATURE_freetype=OFF
$(package)_config_opts_mingw32 += -DQT_FEATURE_ffmpeg=OFF
$(package)_config_opts_mingw32 += -DQT_FEATURE_wmf=ON
$(package)_config_opts_mingw32 += -DBUILD_WITH_PCH=ON

$(package)_config_opts_darwin += -DQT_QMAKE_TARGET_MKSPEC=macx-clang
# see #138
$(package)_config_opts_darwin += -DQT_FEATURE_accessibility=OFF
$(package)_config_opts_darwin += -DQT_FEATURE_dbus=OFF
$(package)_config_opts_darwin += -DQT_FEATURE_freetype=OFF
$(package)_config_opts_darwin += -DQT_FEATURE_ffmpeg=OFF
$(package)_config_opts_darwin += -DQMAKE_MACOSX_DEPLOYMENT_TARGET=11.0
$(package)_config_opts_darwin += -DBUILD_WITH_PCH=OFF
$(package)_config_opts_darwin += '-DQT_QMAKE_DEVICE_OPTIONS=MAC_SDK_PATH=$(host_prefix)/native/SDK;MAC_SDK_VERSION=$(OSX_SDK_VERSION);CROSS_COMPILE=$(host)-;MAC_TARGET=$(host);XCODE_VERSION=$(XCODE_VERSION)'
$(package)_config_opts_darwin += -DQT_NO_APPLE_SDK_AND_XCODE_CHECK=ON
# work around a build issue in qfutex_mac_p.h
$(package)_config_opts_darwin += -DQT_FEATURE_appstore_compliant=ON

$(package)_config_opts += -G Ninja

$(package)_openssl_flags_$(host_os)="-lssl -lcrypto -lpthread -ldl"
$(package)_openssl_flags_mingw32="-lssl -lcrypto -lws2_32"
endef

define $(package)_fetch_cmds
$(call fetch_file,$(package),$($(package)_download_path),$($(package)_download_file),$($(package)_file_name),$($(package)_sha256_hash)) && \
$(call fetch_file,$(package),$($(package)_download_path),$($(package)_qttools_file_name),$($(package)_qttools_file_name),$($(package)_qttools_sha256_hash)) && \
$(call fetch_file,$(package),$($(package)_download_path),$($(package)_qtsvg_file_name),$($(package)_qtsvg_file_name),$($(package)_qtsvg_sha256_hash)) && \
$(call fetch_file,$(package),$($(package)_download_path),$($(package)_qtwebsockets_file_name),$($(package)_qtwebsockets_file_name),$($(package)_qtwebsockets_sha256_hash)) && \
$(call fetch_file,$(package),$($(package)_download_path),$($(package)_qtmultimedia_file_name),$($(package)_qtmultimedia_file_name),$($(package)_qtmultimedia_sha256_hash)) && \
$(call fetch_file,$(package),$($(package)_download_path),$($(package)_qtshadertools_file_name),$($(package)_qtshadertools_file_name),$($(package)_qtshadertools_sha256_hash)) && \
$(call fetch_file,$(package),$($(package)_download_path),$($(package)_qtwayland_file_name),$($(package)_qtwayland_file_name),$($(package)_qtwayland_sha256_hash))
endef

define $(package)_extract_cmds
  mkdir -p $($(package)_extract_dir) && \
  echo "$($(package)_sha256_hash)  $($(package)_source)" > $($(package)_extract_dir)/.$($(package)_file_name).hash && \
  echo "$($(package)_qttools_sha256_hash)  $($(package)_source_dir)/$($(package)_qttools_file_name)" >> $($(package)_extract_dir)/.$($(package)_file_name).hash && \
  echo "$($(package)_qtsvg_sha256_hash)  $($(package)_source_dir)/$($(package)_qtsvg_file_name)" >> $($(package)_extract_dir)/.$($(package)_file_name).hash && \
  echo "$($(package)_qtwebsockets_sha256_hash)  $($(package)_source_dir)/$($(package)_qtwebsockets_file_name)" >> $($(package)_extract_dir)/.$($(package)_file_name).hash && \
  echo "$($(package)_qtmultimedia_sha256_hash)  $($(package)_source_dir)/$($(package)_qtmultimedia_file_name)" >> $($(package)_extract_dir)/.$($(package)_file_name).hash && \
  echo "$($(package)_qtshadertools_sha256_hash)  $($(package)_source_dir)/$($(package)_qtshadertools_file_name)" >> $($(package)_extract_dir)/.$($(package)_file_name).hash && \
  echo "$($(package)_qtwayland_sha256_hash)  $($(package)_source_dir)/$($(package)_qtwayland_file_name)" >> $($(package)_extract_dir)/.$($(package)_file_name).hash && \
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
  $(build_TAR) --no-same-owner --strip-components=1 -xf $($(package)_source_dir)/$($(package)_qtshadertools_file_name) -C qtshadertools && \
  mkdir qtwayland && \
  $(build_TAR) --no-same-owner --strip-components=1 -xf $($(package)_source_dir)/$($(package)_qtwayland_file_name) -C qtwayland
endef

define $(package)_preprocess_cmds
  cp $($(package)_patch_dir)/root_CMakeLists.txt CMakeLists.txt && \
  patch -p1 -i $($(package)_patch_dir)/qtbase-moc-ignore-gcc-macro.patch && \
  patch -p1 -i $($(package)_patch_dir)/rcc_hardcode_timestamp.patch && \
  patch -p1 -i $($(package)_patch_dir)/fast_fixed_dtoa_no_optimize.patch && \
  patch -p1 -i $($(package)_patch_dir)/guix_cross_lib_path.patch && \
  patch -p1 -i $($(package)_patch_dir)/windows_func_fix.patch && \
  mv $($(package)_patch_dir)/toolchain.cmake . && \
  sed -i -e 's|@cmake_system_name@|$($(host_os)_cmake_system)|' \
	     -e 's|@target@|$(host)|' \
	     -e 's|@host_prefix@|$(host_prefix)|' \
	     -e 's|@cmake_c_flags@|$(darwin_CC_)|' \
	     -e 's|@cmake_cxx_flags@|$(darwin_CXX_)|' \
	     -e 's|@cmake_ld_flags@|$(darwin_LDFLAGS)|'\
	     -e 's|@wmf_libs@|$(WMF_LIBS)|' \
      toolchain.cmake && \
  cd qtbase && \
  patch -p1 -i $($(package)_patch_dir)/xcb-util-image-fix.patch && \
  patch -p1 -i $($(package)_patch_dir)/libxau-fix.patch && \
  patch -p1 -i $($(package)_patch_dir)/revert-macOS-Silence-warning-about-supporting-secure.patch && \
  patch -p1 -i $($(package)_patch_dir)/fix_static_qt_darwin_camera_permissions.patch && \
  patch -p1 -i $($(package)_patch_dir)/macos-available-qtbase.patch && \
  cd ../qtmultimedia && \
  patch -p1 -i $($(package)_patch_dir)/qtmultimedia-fixes.patch && \
  patch -p1 -i $($(package)_patch_dir)/v4l2.patch && \
  patch -p1 -i $($(package)_patch_dir)/macos-available-qtmultimedia.patch && \
  cd ../qtwayland && \
  patch -p1 -i $($(package)_patch_dir)/qtwayland-tabletevent-fix.patch
endef


define $(package)_config_cmds
  export OPENSSL_LIBS=${$(package)_openssl_flags_$(host_os)} \
  export PKG_CONFIG_SYSROOT_DIR=/ && \
  export PKG_CONFIG_LIBDIR=$(host_prefix)/lib/pkgconfig && \
  export QT_MAC_SDK_NO_VERSION_CHECK=1 && \
  $($(package)_cmake)
endef

define $(package)_build_cmds
  export LD_LIBRARY_PATH="${build_prefix}/lib/:$(QT_LIBS_LIBS)" && \
  env -u C_INCLUDE_PATH -u CPLUS_INCLUDE_PATH -u OBJC_INCLUDE_PATH -u OBJCPLUS_INCLUDE_PATH -u CPATH -u LIBRARY_PATH cmake --build . --parallel
endef

define $(package)_stage_cmds
  DESTDIR=$($(package)_staging_dir) cmake --install .
endef
