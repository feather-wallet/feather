package=zbar
$(package)_version=0.23.90
$(package)_download_path=https://github.com/mchehab/zbar/archive/refs/tags/
$(package)_download_file=$($(package)_version).tar.gz
$(package)_file_name=$(package)-$($(package)_version).tar.gz
$(package)_sha256_hash=25fdd6726d5c4c6f95c95d37591bfbb2dde63d13d0b10cb1350923ea8b11963b
$(package)_dependencies=libiconv

define $(package)_set_vars
  $(package)_cflags+=-fPIE
  $(package)_cxxflags+=-fPIE
endef

define $(package)_preprocess_cmds
  autoreconf -vfi
endef

define $(package)_set_vars
  $(package)_config_opts=--prefix=$(host_prefix) --disable-shared --without-imagemagick --disable-video --without-xv --with-gtk=no --with-python=no --enable-doc=no --host=$(host)
endef

define $(package)_config_cmds
  $($(package)_autoconf) $($(package)_config_opts)
endef

define $(package)_build_cmds
  $(MAKE) $($(package)_build_opts)
endef

define $(package)_stage_cmds
  $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef