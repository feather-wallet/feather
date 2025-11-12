package=qt
$(package)_version=6.10.0
$(package)_download_path=https://download.qt.io/official_releases/qt/6.10/$($(package)_version)/submodules
$(package)_suffix=everywhere-src-$($(package)_version).tar.xz
$(package)_file_name=qtbase-$($(package)_suffix)
$(package)_sha256_hash=ead4623bcb54a32257c5b3e3a5aec6d16ec96f4cda58d2e003f5a0c16f72046d
$(package)_darwin_dependencies=openssl native_qt
$(package)_mingw32_dependencies=openssl native_qt
$(package)_linux_dependencies=openssl native_qt freetype fontconfig libxcb libxkbcommon libxcb_util libxcb_util_render libxcb_util_keysyms libxcb_util_image libxcb_util_wm libxcb_util_cursor dbus wayland native_wayland
$(package)_patches += qtmultimedia-fixes.patch
$(package)_patches += rcc_hardcode_timestamp.patch
$(package)_patches += root_CMakeLists.txt
$(package)_patches += v4l2.patch
$(package)_patches += windows_func_fix.patch
$(package)_patches += libxau-fix.patch
$(package)_patches += toolchain.cmake
$(package)_patches += no-resonance-audio.patch
$(package)_patches += fix_static_qt_darwin_camera_permissions.patch
#$(package)_patches += fix-static-fontconfig-static-linking.patch

$(package)_qttools_file_name=qttools-$($(package)_suffix)
$(package)_qttools_sha256_hash=d86d5098cf3e3e599f37e18df477e65908fc8f036e10ea731b3469ec4fdbd02a

$(package)_qtsvg_file_name=qtsvg-$($(package)_suffix)
$(package)_qtsvg_sha256_hash=5ed2c0e04d5e73ff75c2a2ed92db5dc1788ba70f704fc2b71bc21644beda2533

$(package)_qtwebsockets_file_name=qtwebsockets-$($(package)_suffix)
$(package)_qtwebsockets_sha256_hash=d1092e7da5c3b8eea242e4069f05ff8e710a17c54d5010c1b557e26f7948806e

$(package)_qtmultimedia_file_name=qtmultimedia-$($(package)_suffix)
$(package)_qtmultimedia_sha256_hash=04424021cf0d1d19799f5967310d484d1afa6fdd0b31725d0ee7608d2eef1126

$(package)_qtshadertools_file_name=qtshadertools-$($(package)_suffix)
$(package)_qtshadertools_sha256_hash=87ba478c3fd827862fc79f6d78d369dfec162c901b7f66ed988b3e1d6ffdfbf6

$(package)_qtwayland_file_name=qtwayland-$($(package)_suffix)
$(package)_qtwayland_sha256_hash=603f2b0a259b24bd0fb14f880d7761b1d248118a42a6870cdbe8fdda4173761f

$(package)_extra_sources += $($(package)_qttools_file_name)
$(package)_extra_sources += $($(package)_qtsvg_file_name)
$(package)_extra_sources += $($(package)_qtwebsockets_file_name)
$(package)_extra_sources += $($(package)_qtmultimedia_file_name)
$(package)_extra_sources += $($(package)_qtshadertools_file_name)
$(package)_extra_sources += $($(package)_qtwayland_file_name)

define $(package)_set_vars
$(package)_config_opts += -DQT_HOST_PATH=$(build_prefix)/qt-host
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
  patch -p1 -i $($(package)_patch_dir)/rcc_hardcode_timestamp.patch && \
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
  patch -p1 -i $($(package)_patch_dir)/libxau-fix.patch && \
  patch -p1 -i $($(package)_patch_dir)/fix_static_qt_darwin_camera_permissions.patch && \
  cd ../qtmultimedia && \
  patch -p1 -i $($(package)_patch_dir)/qtmultimedia-fixes.patch && \
  patch -p1 -i $($(package)_patch_dir)/v4l2.patch
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
