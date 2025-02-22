package=libusb
$(package)_version=1.0.27
$(package)_download_path=https://github.com/libusb/libusb/archive/refs/tags
$(package)_download_file=v$($(package)_version).tar.gz
$(package)_file_name=$(package)-$($(package)_version).tar.gz
$(package)_sha256_hash=e8f18a7a36ecbb11fb820bd71540350d8f61bcd9db0d2e8c18a6fb80b214a3de
$(package)_patches=CMakeLists.txt config.h.in

define $(package)_preprocess_cmds
  cp -f $($(package)_patch_dir)/CMakeLists.txt . && \
  cp -f $($(package)_patch_dir)/config.h.in .
endef

define $(package)_set_vars
  $(package)_config_opts := -DBUILD_SHARED_LIBS=OFF
  $(package)_config_opts_linux += -DLIBUSB_ENABLE_UDEV=OFF
endef

define $(package)_config_cmds
  $($(package)_cmake) .
endef

define $(package)_build_cmd
  $(MAKE)
endef

define $(package)_stage_cmds
  $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef
