package=polyseed
$(package)_version=2.0.0
$(package)_download_path=https://github.com/tevador/polyseed/archive/refs/tags/
$(package)_file_name=v$($(package)_version).tar.gz
$(package)_sha256_hash=f36282fcbcd68d32461b8230c89e1a40661bd46b91109681cec637433004135a
$(package)_patches=force-static-mingw.patch

define $(package)_preprocess_cmds
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
