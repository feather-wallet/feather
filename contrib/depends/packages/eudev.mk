package=eudev
$(package)_version=3.2.12
$(package)_download_path=https://github.com/eudev-project/eudev/releases/download/v$($(package)_version)/
$(package)_file_name=eudev-$($(package)_version).tar.gz
$(package)_sha256_hash=ccdd64ec3c381d3c3ed0e99d2e70d1f62988c7763de89ca7bdffafa5eacb9ad8

define $(package)_set_vars
  $(package)_config_opts=--disable-gudev --disable-introspection --disable-hwdb --disable-manpages --disable-shared
endef

define $(package)_config_cmds
  $($(package)_autoconf)
endef

define $(package)_build_cmd
  $(MAKE)
endef

define $(package)_preprocess_cmds
  cd $($(package)_build_subdir); autoreconf -f -i
endef

define $(package)_stage_cmds
  $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef

define $(package)_postprocess_cmds
  rm lib/*.la
endef
