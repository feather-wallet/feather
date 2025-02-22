package=libusb
$(package)_version=1.0.27
$(package)_download_path=https://github.com/libusb/libusb/archive/refs/tags
$(package)_download_file=v$($(package)_version).tar.gz
$(package)_file_name=$(package)-$($(package)_version).tar.gz
$(package)_sha256_hash=e8f18a7a36ecbb11fb820bd71540350d8f61bcd9db0d2e8c18a6fb80b214a3de
$(package)_patches=fix-c11-check.patch

define $(package)_preprocess_cmds
  patch -p1 < $($(package)_patch_dir)/fix-c11-check.patch && \
  cp -f $(BASEDIR)/config.guess $(BASEDIR)/config.sub . && \
  autoreconf -i
endef

define $(package)_set_vars
  $(package)_config_opts=--disable-shared
  $(package)_config_opts_linux=--with-pic --disable-udev
  $(package)_config_opts_mingw32=--disable-udev
  $(package)_config_opts_darwin=--disable-udev
endef

define $(package)_config_cmds
  $($(package)_autoconf)
endef

define $(package)_build_cmd
  $(MAKE)
endef

define $(package)_stage_cmds
  $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef

define $(package)_postprocess_cmds
  cp -f lib/libusb-1.0.a lib/libusb.a
endef
