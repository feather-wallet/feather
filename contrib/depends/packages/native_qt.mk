package=native_qt
$(package)_version=6.6.0
$(package)_download_path=https://download.qt.io/official_releases/qt/6.6/$($(package)_version)/submodules
$(package)_suffix=everywhere-src-$($(package)_version).tar.xz
$(package)_file_name=qtbase-$($(package)_suffix)
$(package)_sha256_hash=039d53312acb5897a9054bd38c9ccbdab72500b71fdccdb3f4f0844b0dd39e0e
$(package)_qt_libs=corelib network widgets gui plugins testlib
$(package)_patches  = dont_hardcode_pwd.patch
$(package)_patches += fast_fixed_dtoa_no_optimize.patch
$(package)_patches += guix_cross_lib_path.patch
$(package)_patches += qtbase-moc-ignore-gcc-macro.patch
$(package)_patches += rcc_hardcode_timestamp.patch
$(package)_patches += root_CMakeLists.txt

$(package)_qttools_file_name=qttools-$($(package)_suffix)
$(package)_qttools_sha256_hash=4e9feebc142bbb6e453e1dc3277e09ec45c8ef081b5ee2a029e6684b5905ba99

$(package)_qtsvg_file_name=qtsvg-$($(package)_suffix)
$(package)_qtsvg_sha256_hash=33da25fef51102f564624a7ea3e57cb4a0a31b7b44783d1af5749ac36d3c72de

$(package)_qtmultimedia_file_name=qtmultimedia-$($(package)_suffix)
$(package)_qtmultimedia_sha256_hash=ba4b236ca3ec118841b7f5ba87797e24a8c5cbef9ba973fe219fbfc310d38782

$(package)_qtshadertools_file_name=qtshadertools-$($(package)_suffix)
$(package)_qtshadertools_sha256_hash=3690327dc550144b8102be2bb7f6dc995ccbe3584041094ad021a83737ef9875

$(package)_extra_sources += $($(package)_qttools_file_name)
$(package)_extra_sources += $($(package)_qtsvg_file_name)
$(package)_extra_sources += $($(package)_qtmultimedia_file_name)
$(package)_extra_sources += $($(package)_qtshadertools_file_name)

define $(package)_set_vars
$(package)_config_opts_release = -release
$(package)_config_opts_debug = -debug
$(package)_config_opts_debug += -optimized-tools
$(package)_config_opts += -libexecdir $(build_prefix)/qt-host/bin
$(package)_config_opts += -c++std c++17
$(package)_config_opts += -confirm-license
$(package)_config_opts += -opensource
$(package)_config_opts += -pkg-config
$(package)_config_opts += -prefix $(build_prefix)/qt-host
$(package)_config_opts += -static
$(package)_config_opts += -platform linux-g++ -xplatform linux-g++
ifneq (,$(findstring -stdlib=libc++,$($(1)_cxx)))
$(package)_config_opts_x86_64 = -xplatform linux-clang-libc++
endif

$(package)_config_opts += -no-cups
$(package)_config_opts += -no-egl
$(package)_config_opts += -no-eglfs
$(package)_config_opts += -no-evdev
$(package)_config_opts += -no-feature-androiddeployqt
$(package)_config_opts += -no-feature-colordialog
$(package)_config_opts += -no-feature-dial
$(package)_config_opts += -no-feature-fontcombobox
$(package)_config_opts += -no-feature-fontconfig
$(package)_config_opts += -no-feature-freetype
$(package)_config_opts += -no-feature-harfbuzz
$(package)_config_opts += -no-feature-http
$(package)_config_opts += -no-feature-image_heuristic_mask
$(package)_config_opts += -no-feature-keysequenceedit
$(package)_config_opts += -no-feature-lcdnumber
$(package)_config_opts += -no-feature-networkdiskcache
$(package)_config_opts += -no-feature-pdf
$(package)_config_opts += -no-feature-png
$(package)_config_opts += -no-feature-printdialog
$(package)_config_opts += -no-feature-printer
$(package)_config_opts += -no-feature-printpreviewdialog
$(package)_config_opts += -no-feature-printpreviewwidget
$(package)_config_opts += -no-feature-qmake
$(package)_config_opts += -no-feature-sessionmanager
$(package)_config_opts += -no-feature-spatialaudio
$(package)_config_opts += -no-feature-sql
$(package)_config_opts += -no-feature-syntaxhighlighter
$(package)_config_opts += -no-feature-testlib
$(package)_config_opts += -no-feature-textmarkdownwriter
$(package)_config_opts += -no-feature-textodfwriter
$(package)_config_opts += -no-feature-topleveldomain
$(package)_config_opts += -no-feature-undocommand
$(package)_config_opts += -no-feature-undogroup
$(package)_config_opts += -no-feature-undostack
$(package)_config_opts += -no-feature-undoview
$(package)_config_opts += -no-feature-vnc
$(package)_config_opts += -no-feature-vulkan
$(package)_config_opts += -no-feature-widgets
$(package)_config_opts += -no-feature-xkbcommon
$(package)_config_opts += -no-feature-xlib
$(package)_config_opts += -no-gif
$(package)_config_opts += -no-glib
$(package)_config_opts += -no-ico
$(package)_config_opts += -no-icu
$(package)_config_opts += -no-kms
$(package)_config_opts += -no-libjpeg
$(package)_config_opts += -no-libudev
$(package)_config_opts += -no-linuxfb
$(package)_config_opts += -nomake examples
$(package)_config_opts += -nomake tests
$(package)_config_opts += -no-mtdev
$(package)_config_opts += -no-opengl
$(package)_config_opts += -no-openssl
$(package)_config_opts += -no-openvg
$(package)_config_opts += -no-pch
$(package)_config_opts += -no-reduce-relocations
$(package)_config_opts += -no-schannel
$(package)_config_opts += -no-sctp
$(package)_config_opts += -no-securetransport
$(package)_config_opts += -no-system-proxies
$(package)_config_opts += -no-use-gold-linker
$(package)_config_opts += -no-xcb-xlib
$(package)_config_opts += -no-zstd

ifneq ($(LTO),)
$(package)_config_opts += -ltcg
endif
endef

define $(package)_fetch_cmds
$(call fetch_file,$(package),$($(package)_download_path),$($(package)_download_file),$($(package)_file_name),$($(package)_sha256_hash)) && \
$(call fetch_file,$(package),$($(package)_download_path),$($(package)_qttools_file_name),$($(package)_qttools_file_name),$($(package)_qttools_sha256_hash)) && \
$(call fetch_file,$(package),$($(package)_download_path),$($(package)_qtsvg_file_name),$($(package)_qtsvg_file_name),$($(package)_qtsvg_sha256_hash)) && \
$(call fetch_file,$(package),$($(package)_download_path),$($(package)_qtmultimedia_file_name),$($(package)_qtmultimedia_file_name),$($(package)_qtmultimedia_sha256_hash)) && \
$(call fetch_file,$(package),$($(package)_download_path),$($(package)_qtshadertools_file_name),$($(package)_qtshadertools_file_name),$($(package)_qtshadertools_sha256_hash))
endef

define $(package)_extract_cmds
  mkdir -p $($(package)_extract_dir) && \
  echo "$($(package)_sha256_hash)  $($(package)_source)" > $($(package)_extract_dir)/.$($(package)_file_name).hash && \
  echo "$($(package)_qttools_sha256_hash)  $($(package)_source_dir)/$($(package)_qttools_file_name)" >> $($(package)_extract_dir)/.$($(package)_file_name).hash && \
  echo "$($(package)_qtsvg_sha256_hash)  $($(package)_source_dir)/$($(package)_qtsvg_file_name)" >> $($(package)_extract_dir)/.$($(package)_file_name).hash && \
  echo "$($(package)_qtmultimedia_sha256_hash)  $($(package)_source_dir)/$($(package)_qtmultimedia_file_name)" >> $($(package)_extract_dir)/.$($(package)_file_name).hash && \
  echo "$($(package)_qtshadertools_sha256_hash)  $($(package)_source_dir)/$($(package)_qtshadertools_file_name)" >> $($(package)_extract_dir)/.$($(package)_file_name).hash && \
  $(build_SHA256SUM) -c $($(package)_extract_dir)/.$($(package)_file_name).hash && \
  mkdir qtbase && \
  $(build_TAR) --no-same-owner --strip-components=1 -xf $($(package)_source) -C qtbase && \
  mkdir qttools && \
  $(build_TAR) --no-same-owner --strip-components=1 -xf $($(package)_source_dir)/$($(package)_qttools_file_name) -C qttools && \
  mkdir qtsvg && \
  $(build_TAR) --no-same-owner --strip-components=1 -xf $($(package)_source_dir)/$($(package)_qtsvg_file_name) -C qtsvg && \
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
  patch -p1 -i $($(package)_patch_dir)/guix_cross_lib_path.patch
endef

define $(package)_config_cmds
  export PKG_CONFIG_SYSROOT_DIR=/ && \
  export PKG_CONFIG_LIBDIR=$(build_prefix)/lib/pkgconfig && \
  export QT_MAC_SDK_NO_VERSION_CHECK=1 && \
  unset CMAKE_PREFIX_PATH && \
  export CMAKE_PREFIX_PATH="$(QT_LIBS)" && \
  cd qtbase && \
  ./configure -top-level $($(package)_config_opts)
endef

define $(package)_build_cmds
  export LD_LIBRARY_PATH=${build_prefix}/lib/ && \
  unset CMAKE_PREFIX_PATH && \
  cmake --build . --parallel
endef

define $(package)_stage_cmds
  DESTDIR=$($(package)_staging_dir) cmake --install .
endef
