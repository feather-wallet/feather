package=libgcrypt
$(package)_version=1.11.0
$(package)_download_path=https://www.gnupg.org/ftp/gcrypt/libgcrypt/
$(package)_file_name=libgcrypt-$($(package)_version).tar.gz
$(package)_sha256_hash=2382891207d3b000b20c81dbf2036516a535d31abd80f57d455e711e1dde5ff5
$(package)_dependencies=libgpg-error
$(package)_patches=no-programs.patch

define $(package)_set_vars
   $(package)_build_opts=CFLAGS="-fPIE"
endef

define $(package)_preprocess_cmds
    patch -p1 < $($(package)_patch_dir)/no-programs.patch && \
    cp -f $(BASEDIR)/config.guess $(BASEDIR)/config.sub build-aux
endef

define $(package)_config_cmds
    $($(package)_autoconf) --disable-shared --enable-static --disable-tests --disable-doc --disable-asm --with-libgpg-error-prefix=$(host_prefix)
endef

define $(package)_build_cmds
    $(MAKE) $($(package)_build_opts)
endef

define $(package)_stage_cmds
    $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef
