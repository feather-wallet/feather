package=libzip
$(package)_version=1.10.0
$(package)_download_path=https://libzip.org/download/
$(package)_file_name=libzip-1.10.0.tar.xz
$(package)_sha256_hash=cd2a7ac9f1fb5bfa6218272d9929955dc7237515bba6e14b5ad0e1d1e2212b43
$(package)_dependencies=zlib
$(package)_patches += no-clonefile.patch

define $(package)_preprocess_cmds
    patch -p1 < $($(package)_patch_dir)/no-clonefile.patch
endef

define $(package)_config_cmds
    $($(package)_cmake) -DZLIB_ROOT=$(host_prefix) -DENABLE_BZIP2=Off -DBUILD_SHARED_LIBS=OFF -DCMAKE_INSTALL_PREFIX=$(host_prefix) .
endef

define $(package)_build_cmds
    $(MAKE)
endef

define $(package)_stage_cmds
    $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef