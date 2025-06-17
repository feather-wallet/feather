package=libffi
$(package)_version=$(native_$(package)_version)
$(package)_download_path := $(native_$(package)_download_path)
$(package)_file_name := $(native_$(package)_file_name)
$(package)_sha256_hash := $(native_$(package)_sha256_hash)

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
