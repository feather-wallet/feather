package=quirc
$(package)_version=1.2
$(package)_download_path=https://github.com/dlbeer/quirc/archive/refs/tags/
$(package)_file_name=v1.2.tar.gz
$(package)_sha256_hash=73c12ea33d337ec38fb81218c7674f57dba7ec0570bddd5c7f7a977c0deb64c5
$(package)_patches += CMakeLists.txt

define $(package)_preprocess_cmds
  cp $($(package)_patch_dir)/CMakeLists.txt CMakeLists.txt
endef

define $(package)_config_cmds
    $($(package)_cmake) -DCMAKE_INSTALL_PREFIX=$(host_prefix) .
endef

define $(package)_build_cmds
    $(MAKE)
endef

define $(package)_stage_cmds
    $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef
