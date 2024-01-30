package=zlib
$(package)_version=1.3.1
$(package)_download_path=https://github.com/madler/zlib/releases/download/v$($(package)_version)/
$(package)_file_name=$(package)-$($(package)_version).tar.xz
$(package)_sha256_hash=38ef96b8dfe510d42707d9c781877914792541133e1870841463bfa73f883e32

define $(package)_config_cmds
    CC="$($(package)_cc)" \
    CXX="$($(package)_cxx)" \
    AR="$($(package)_ar)" \
    RANLIB="$($(package)_ranlib)" \
    LIBTOOL="$($(package)_libtool)" \
    LDLAGS="$($(package)_ldflags)" \
    CFLAGS="-fPIE" \
    CXXFLAGS="-fPIE" \
	./configure --static --prefix=$(host_prefix)
endef

define $(package)_build_cmds
    $(MAKE)
endef

define $(package)_stage_cmds
    $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef
