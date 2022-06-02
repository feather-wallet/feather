package=libgcrypt
$(package)_version=1.8.9
$(package)_download_path=https://www.gnupg.org/ftp/gcrypt/libgcrypt/
$(package)_file_name=libgcrypt-$($(package)_version).tar.bz2
$(package)_sha256_hash=2bda4790aa5f0895d3407cf7bf6bd7727fd992f25a45a63d92fef10767fa3769
$(package)_dependencies=libgpg-error
$(package)_patches=gost-sb.h no_gen_gost-sb.patch

define $(package)_set_vars
  $(package)_build_opts=CFLAGS="-fPIE"
endef

define $(package)_preprocess_cmds
    mv $($(package)_patch_dir)/gost-sb.h cipher/gost-sb.h && \
    patch -p1 < $($(package)_patch_dir)/no_gen_gost-sb.patch
endef

# building on linux with $($(package)_autoconf) fails for mysterious reasons
ifeq ($(host_os),linux)
define $(package)_config_cmds
    CLAGS='-fPIE' CXXFLAGS='-fPIE' ./configure --disable-shared --enable-static --disable-doc --with-libgpg-error-prefix=$(host_prefix) --prefix=$(host_prefix)
endef
else
define $(package)_config_cmds
    $($(package)_autoconf) --disable-shared --enable-static --disable-doc --with-libgpg-error-prefix=$(host_prefix)
endef
endif

define $(package)_build_cmds
    $(MAKE) $($(package)_build_opts)
endef

define $(package)_stage_cmds
    $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef
