package=hidapi
$(package)_version=0.14.0
$(package)_download_path=https://github.com/libusb/hidapi/archive/refs/tags
$(package)_file_name=$(package)-$($(package)_version).tar.gz
$(package)_sha256_hash=a5714234abe6e1f53647dd8cba7d69f65f71c558b7896ed218864ffcf405bcbd
$(package)_linux_dependencies=libusb
$(package)_patches=cmake-fix-libusb.patch

define $(package)_set_vars
  $(package)_config_opts := -DBUILD_SHARED_LIBS=OFF
  $(package)_config_opts += -DHIDAPI_WITH_HIDRAW=OFF
endef

define $(package)_preprocess_cmds
  patch -p1 < $($(package)_patch_dir)/cmake-fix-libusb.patch
endef

define $(package)_config_cmds
  $($(package)_cmake) .
endef

define $(package)_build_cmds
  $(MAKE)
endef

define $(package)_stage_cmds
  $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef
