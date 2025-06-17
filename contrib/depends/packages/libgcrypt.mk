package=libgcrypt
$(package)_version=1.11.1
$(package)_download_path=https://www.gnupg.org/ftp/gcrypt/libgcrypt/
$(package)_file_name=libgcrypt-$($(package)_version).tar.gz
$(package)_sha256_hash=a9691689e5e2f3be03c90738c9ab1d194245d82e365fb3797f3b0fd04de45d7b
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
