package=zxing-cpp
$(package)_version=2.3.0
$(package)_download_path=https://github.com/$(package)/$(package)/archive/refs/tags
$(package)_download_file=v$($(package)_version).tar.gz
$(package)_file_name=$(package)-$($(package)_version).tar.gz
$(package)_sha256_hash=64e4139103fdbc57752698ee15b5f0b0f7af9a0331ecbdc492047e0772c417ba

define $(package)_set_vars
  $(package)_config_opts=-DBUILD_WRITERS=OFF -DBUILD_EXAMPLES=OFF -DBUILD_SHARED_LIBS=OFF
endef

define $(package)_preprocess_cmds
  rm -rf test wrappers
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
