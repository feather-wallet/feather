package=libarchive
$(package)_version=3.7.4
$(package)_download_path=https://github.com/libarchive/libarchive/releases/download/v$($(package)_version)/
$(package)_file_name=$(package)-$($(package)_version).tar.xz
$(package)_sha256_hash=f887755c434a736a609cbd28d87ddbfbe9d6a3bb5b703c22c02f6af80a802735

define $(package)_config_cmds
    CC="$($(package)_cc)" \
    CXX="$($(package)_cxx)" \
    AR="$($(package)_ar)" \
    RANLIB="$($(package)_ranlib)" \
    LIBTOOL="$($(package)_libtool)" \
    LDLAGS="$($(package)_ldflags)" \
    CFLAGS="-fPIE" \
    CXXFLAGS="-fPIE" \
	./configure --host=$(host) --enable-static --prefix=$(host_prefix) --without-iconv
endef

define $(package)_build_cmds
    $(MAKE)
endef

define $(package)_stage_cmds
    $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef
