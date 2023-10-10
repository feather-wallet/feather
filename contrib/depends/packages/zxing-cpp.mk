package=zxing-cpp
$(package)_version=2.1.0
$(package)_download_path=https://github.com/$(package)/$(package)/archive/refs/tags
$(package)_download_file=v$($(package)_version).tar.gz
$(package)_file_name=$(package)-$($(package)_version).tar.gz
$(package)_sha256_hash=6d54e403592ec7a143791c6526c1baafddf4c0897bb49b1af72b70a0f0c4a3fe

define $(package)_set_vars
  $(package)_config_opts=-DBUILD_WRITERS=OFF -DBUILD_EXAMPLES=OFF -DBUILD_SHARED_LIBS=OFF
endef

define $(package)_config_cmds
  $($(package)_cmake) .
endef

define $(package)_build_cmds
  $(MAKE)
endef

define $(package)_stage_cmds
  $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef
