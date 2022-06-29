package=polyseed
$(package)_version=1.0.0
$(package)_download_path=https://github.com/tevador/polyseed/archive/refs/tags/
$(package)_file_name=v$($(package)_version).tar.gz
$(package)_sha256_hash=45f1e6c08575286581079e6e26d341a3a33abe1f1ee2d026bd098cf632ea2349
$(package)_dependencies=native_cmake
$(package)_patches=no_shared.patch force-static-mingw.patch

define $(package)_preprocess_cmds
    patch -p1 < $($(package)_patch_dir)/no_shared.patch && \
    patch -p1 < $($(package)_patch_dir)/force-static-mingw.patch
endef

define $(package)_config_cmds
    $($(package)_cmake) -DCMAKE_INSTALL_PREFIX=$(host_prefix) -DCMAKE_C_COMPILER= .
endef

define $(package)_build_cmds
    $(MAKE)
endef

define $(package)_stage_cmds
    $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef
