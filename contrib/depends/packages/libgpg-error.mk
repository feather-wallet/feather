package=libgpg-error
$(package)_version=1.49
$(package)_download_path=https://www.gnupg.org/ftp/gcrypt/libgpg-error/
$(package)_file_name=libgpg-error-$($(package)_version).tar.gz
$(package)_sha256_hash=e59cc3ced0ae86f49073e2f2344676919a82fc5033716bee7232f6f778158792

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
