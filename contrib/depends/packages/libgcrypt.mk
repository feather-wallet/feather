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

# TODO: do a native compile first to eliminate the need for this patch
define $(package)_preprocess_cmds
    mv $($(package)_patch_dir)/gost-sb.h cipher/gost-sb.h && \
    patch -p1 < $($(package)_patch_dir)/no_gen_gost-sb.patch
endef

# TODO: building on linux with $($(package)_autoconf) fails for mysterious reasons
ifeq ($(host_os),linux)
define $(package)_config_cmds
    CFLAGS='-fPIE' CXXFLAGS='-fPIE' ./configure --host=$(host) --enable-digests="sha256 blake2" --enable-ciphers=aes --disable-amd64-as-feature-detection --disable-asm --disable-avx-support --disable-avx2-support --disable-sse41-support --disable-shared --enable-static --disable-doc --with-libgpg-error-prefix=$(host_prefix) --prefix=$(host_prefix)
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
