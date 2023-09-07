package=libzip
$(package)_version=1.10.1
$(package)_download_path=https://libzip.org/download/
$(package)_file_name=$(package)-$($(package)_version).tar.xz
$(package)_sha256_hash=dc3c8d5b4c8bbd09626864f6bcf93de701540f761d76b85d7c7d710f4bd90318
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