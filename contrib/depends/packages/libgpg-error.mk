package=libgpg-error
$(package)_version=1.50
$(package)_download_path=https://www.gnupg.org/ftp/gcrypt/libgpg-error/
$(package)_file_name=libgpg-error-$($(package)_version).tar.gz
$(package)_sha256_hash=34a3b36ec8ad830f8f3ceb5db583d1f6dc8ae4c31d04f6da18ea18dd95396ab0
$(package)_patches=declare_environ_macos.patch

define $(package)_set_vars
  $(package)_config_opts := --enable-static --disable-shared
  $(package)_config_opts += --disable-doc --disable-tests
  $(package)_config_opts += --enable-install-gpg-error-config
  $(package)_build_opts := CFLAGS="-fPIE"
endef

define $(package)_preprocess_cmds
  patch -p1 < $($(package)_patch_dir)/declare_environ_macos.patch && \
  cp -f $(BASEDIR)/config.guess $(BASEDIR)/config.sub build-aux
endef

define $(package)_config_cmds
  $($(package)_autoconf)
endef

define $(package)_build_cmds
  $(MAKE) $($(package)_build_opts)
endef

define $(package)_stage_cmds
  $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef
