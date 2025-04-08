package=libusb
$(package)_version=1.0.28
$(package)_download_path=https://github.com/libusb/libusb/archive/refs/tags
$(package)_download_file=v$($(package)_version).tar.gz
$(package)_file_name=$(package)-$($(package)_version).tar.gz
$(package)_sha256_hash=378b3709a405065f8f9fb9f35e82d666defde4d342c2a1b181a9ac134d23c6fe
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
