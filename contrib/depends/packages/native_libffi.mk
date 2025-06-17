package=native_libffi
$(package)_version=3.5.1
$(package)_download_path := https://github.com/libffi/libffi/releases/download/v$($(package)_version)
$(package)_file_name := libffi-$($(package)_version).tar.gz
$(package)_sha256_hash := f99eb68a67c7d54866b7706af245e87ba060d419a062474b456d3bc8d4abdbd1

define $(package)_set_vars
  $(package)_config_opts := --enable-option-checking --disable-dependency-tracking
  $(package)_config_opts += --enable-shared --disable-static --disable-docs
  $(package)_config_opts += --disable-multi-os-directory
endef

define $(package)_config_cmds
  $($(package)_autoconf)
endef

define $(package)_build_cmds
  $(MAKE)
endef

define $(package)_stage_cmds
  $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef

define $(package)_postprocess_cmds
  rm -rf share
endef
