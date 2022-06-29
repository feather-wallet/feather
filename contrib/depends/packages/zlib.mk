package=zlib
$(package)_version=1.2.12
$(package)_download_path=https://www.zlib.net/
$(package)_file_name=zlib-1.2.12.tar.gz
$(package)_sha256_hash=91844808532e5ce316b3c010929493c0244f3d37593afd6de04f71821d5136d9

ifeq ($(host_os),linux)
define $(package)_config_cmds
    LDLAGS="$($(package)_ldflags)" ./configure --static --prefix=$(host_prefix)
endef
else
define $(package)_config_cmds
    CC="$($(package)_cc)" \
    CXX="$($(package)_cxx)" \
    AR="$($(package)_ar)" \
    RANLIB="$($(package)_ranlib)" \
    LIBTOOL="$($(package)_libtool)" \
    LDLAGS="$($(package)_ldflags)" \
	./configure --static --prefix=$(host_prefix)
endef
endif

define $(package)_build_cmds
    $(MAKE)
endef

define $(package)_stage_cmds
    $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef