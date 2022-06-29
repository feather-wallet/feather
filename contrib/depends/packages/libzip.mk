package=libzip
$(package)_version=1.8.0
$(package)_download_path=https://libzip.org/download/
$(package)_file_name=libzip-1.8.0.tar.gz
$(package)_sha256_hash=30ee55868c0a698d3c600492f2bea4eb62c53849bcf696d21af5eb65f3f3839e
$(package)_dependencies=zlib native_cmake

define $(package)_config_cmds
    $($(package)_cmake) -DBUILD_SHARED_LIBS=OFF -DCMAKE_INSTALL_PREFIX=$(host_prefix) .
endef

define $(package)_build_cmds
    $(MAKE)
endef

define $(package)_stage_cmds
    $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef