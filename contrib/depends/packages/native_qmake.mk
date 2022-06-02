package=native_qmake
$(package)_version=5.15.3
$(package)_download_path=https://download.qt.io/official_releases/qt/5.15/$($(package)_version)/submodules
$(package)_suffix=everywhere-opensource-src-$($(package)_version).tar.xz
$(package)_file_name=qtbase-$($(package)_suffix)
$(package)_sha256_hash=26394ec9375d52c1592bd7b689b1619c6b8dbe9b6f91fdd5c355589787f3a0b6
$(package)_linux_dependencies=freetype fontconfig libxcb libxkbcommon libxcb_util libxcb_util_render libxcb_util_keysyms libxcb_util_image libxcb_util_wm
$(package)_patches += no-xlib.patch

define $(package)_set_vars
$(package)_config_opts_release = -release
$(package)_config_opts += -bindir $(build_prefix)/bin
$(package)_config_opts += -c++std c++17
$(package)_config_opts += -confirm-license
$(package)_config_opts += -hostprefix $(build_prefix)
$(package)_config_opts += -opensource
$(package)_config_opts += -prefix $(host_prefix)
$(package)_config_opts += -v
$(package)_config_opts_linux = -xcb
$(package)_config_opts_linux += -no-xcb-xlib
$(package)_config_opts_linux += -no-feature-xlib
$(package)_config_opts_linux += -no-opengl
$(package)_config_opts_linux += -no-feature-vulkan
$(package)_config_opts_linux += -dbus-runtime
endef

define $(package)_fetch_cmds
$(call fetch_file,$(package),$($(package)_download_path),$($(package)_download_file),$($(package)_file_name),$($(package)_sha256_hash))
endef

define $(package)_extract_cmds
  mkdir -p $($(package)_extract_dir) && \
  echo "$($(package)_sha256_hash)  $($(package)_source)" > $($(package)_extract_dir)/.$($(package)_file_name).hash && \
  $(build_SHA256SUM) -c $($(package)_extract_dir)/.$($(package)_file_name).hash && \
  mkdir qtbase && \
  $(build_TAR) --no-same-owner --strip-components=1 -xf $($(package)_source) -C qtbase
endef

define $(package)_preprocess_cmds
  patch -p1 -i $($(package)_patch_dir)/no-xlib.patch
endef

define $(package)_config_cmds
  cd qtbase && \
  ./configure $($(package)_config_opts)
endef

define $(package)_build_cmds
  cd qtbase && \
  $(MAKE)
endef

define $(package)_stage_cmds
  cd qtbase && \
  $(MAKE) INSTALL_ROOT=$($(package)_staging_dir) install
endef

define $(package)_postprocess_cmds
    echo -n "" > lib/cmake/Qt5Core/Qt5CoreConfigExtras.cmake
endef
