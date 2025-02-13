package=libffi
$(package)_version=3.4.7
$(package)_download_path := https://github.com/libffi/$(package)/releases/download/v$($(package)_version)
$(package)_file_name := libffi-$($(package)_version).tar.gz
$(package)_sha256_hash := 76653f135dde3036036c500e11861648ffbf9e1fc5b233ff473c60897d9db0ea

define $(package)_set_vars
  $(package)_config_opts := --enable-option-checking --disable-dependency-tracking
  $(package)_config_opts += --disable-shared --enable-static --disable-docs
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
