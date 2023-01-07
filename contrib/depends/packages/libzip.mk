package=libzip
$(package)_version=1.9.2
$(package)_download_path=https://libzip.org/download/
$(package)_file_name=libzip-1.9.2.tar.gz
$(package)_sha256_hash=fd6a7f745de3d69cf5603edc9cb33d2890f0198e415255d0987a0cf10d824c6f
$(package)_dependencies=zlib native_cmake
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