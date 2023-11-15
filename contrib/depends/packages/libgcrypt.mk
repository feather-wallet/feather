package=libgcrypt
$(package)_version=1.10.3
$(package)_download_path=https://www.gnupg.org/ftp/gcrypt/libgcrypt/
$(package)_file_name=libgcrypt-$($(package)_version).tar.bz2
$(package)_sha256_hash=8b0870897ac5ac67ded568dcfadf45969cfa8a6beb0fd60af2a9eadc2a3272aa
$(package)_dependencies=libgpg-error

define $(package)_set_vars
   $(package)_build_opts=CFLAGS="-fPIE"
endef

define $(package)_preprocess_cmds
    cp -f $(BASEDIR)/config.guess $(BASEDIR)/config.sub build-aux
endef

# TODO: building on linux with $($(package)_autoconf) fails for mysterious reasons
ifeq ($(host_os),linux)
define $(package)_config_cmds
    CFLAGS='-fPIE' CXXFLAGS='-fPIE' ./configure --host=$(host) --enable-digests="sha256 blake2" --enable-ciphers="aes chacha20" --disable-amd64-as-feature-detection --disable-asm --disable-avx-support --disable-avx2-support --disable-sse41-support --disable-shared --enable-static --disable-doc --with-libgpg-error-prefix=$(host_prefix) --prefix=$(host_prefix)
endef
else
define $(package)_config_cmds
    $($(package)_autoconf) --disable-shared --enable-static --disable-doc --disable-asm --with-libgpg-error-prefix=$(host_prefix)
endef
endif

define $(package)_build_cmds
    $(MAKE) $($(package)_build_opts)
endef

define $(package)_stage_cmds
    $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef
