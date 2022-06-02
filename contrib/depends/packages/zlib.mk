package=zlib
$(package)_version=1.2.11
$(package)_download_path=https://github.com/madler/zlib/archive/refs/tags/
$(package)_file_name=v1.2.11.tar.gz
$(package)_sha256_hash=629380c90a77b964d896ed37163f5c3a34f6e6d897311f1df2a7016355c45eff

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