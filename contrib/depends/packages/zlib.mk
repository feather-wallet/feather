package=zlib
$(package)_version=1.2.13
$(package)_download_path=https://github.com/madler/zlib/releases/download/v$($(package)_version)/
$(package)_file_name=$(package)-$($(package)_version).tar.gz
$(package)_sha256_hash=b3a24de97a8fdbc835b9833169501030b8977031bcb54b3b3ac13740f846ab30

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