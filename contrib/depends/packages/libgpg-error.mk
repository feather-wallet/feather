package=libgpg-error
$(package)_version=1.47
$(package)_download_path=https://www.gnupg.org/ftp/gcrypt/libgpg-error/
$(package)_file_name=libgpg-error-$($(package)_version).tar.bz2
$(package)_sha256_hash=9e3c670966b96ecc746c28c2c419541e3bcb787d1a73930f5e5f5e1bcbbb9bdb

define $(package)_set_vars
  $(package)_build_opts=CFLAGS="-fPIE"
endef

define $(package)_preprocess_cmds
  cp -f $(BASEDIR)/config.guess $(BASEDIR)/config.sub build-aux
endef

define $(package)_config_cmds
    $($(package)_autoconf) --enable-static --disable-shared --enable-install-gpg-error-config
endef

define $(package)_build_cmds
    $(MAKE) $($(package)_build_opts)
endef

define $(package)_stage_cmds
    $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef
