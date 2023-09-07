package=zlib
$(package)_version=1.3
$(package)_download_path=https://github.com/madler/zlib/releases/download/v$($(package)_version)/
$(package)_file_name=$(package)-$($(package)_version).tar.xz
$(package)_sha256_hash=8a9ba2898e1d0d774eca6ba5b4627a11e5588ba85c8851336eb38de4683050a7

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