package=libgpg-error
$(package)_version=1.48
$(package)_download_path=https://www.gnupg.org/ftp/gcrypt/libgpg-error/
$(package)_file_name=libgpg-error-$($(package)_version).tar.bz2
$(package)_sha256_hash=89ce1ae893e122924b858de84dc4f67aae29ffa610ebf668d5aa539045663d6f

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
