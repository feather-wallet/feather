package=libgpg-error
$(package)_version=1.45
$(package)_download_path=https://www.gnupg.org/ftp/gcrypt/libgpg-error/
$(package)_file_name=libgpg-error-1.45.tar.bz2
$(package)_sha256_hash=570f8ee4fb4bff7b7495cff920c275002aea2147e9a1d220c068213267f80a26

define $(package)_set_vars
  $(package)_build_opts=CFLAGS="-fPIE"
endef

define $(package)_config_cmds
    $($(package)_autoconf) --enable-static --disable-shared
endef

define $(package)_build_cmds
    $(MAKE) $($(package)_build_opts)
endef

define $(package)_stage_cmds
    $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef
