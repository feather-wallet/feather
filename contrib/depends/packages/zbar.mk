package=zbar
$(package)_version=0.23.92
$(package)_download_path=https://github.com/mchehab/zbar/archive/refs/tags/
$(package)_file_name=$($(package)_version).tar.gz
$(package)_sha256_hash=dffc16695cb6e42fa318a4946fd42866c0f5ab735f7eaf450b108d1c3a19b4ba
$(package)_dependencies=libiconv

define $(package)_set_vars
  $(package)_build_opts=CFLAGS="-fPIE"
  $(package)_build_opts+=CXXFLAGS="-fPIE"
endef

define $(package)_preprocess_cmds
    autoreconf -vfi
endef

define $(package)_set_vars
    $(package)_config_opts=--prefix=$(host_prefix) --disable-shared --without-imagemagick --disable-video --without-xv --with-gtk=no --with-python=no --enable-doc=no --host=$(host)
endef

define $(package)_config_cmds
    CFLAGS="-fPIE" CXXFLAGS="-fPIE" $($(package)_autoconf) $($(package)_config_opts)
endef

define $(package)_build_cmds
    CFLAGS="-fPIE" CXXFLAGS="-fPIE" $(MAKE) $($(package)_build_opts)
endef

define $(package)_stage_cmds
  $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef