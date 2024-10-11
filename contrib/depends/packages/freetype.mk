package=freetype
$(package)_version=2.13.3
$(package)_download_path=https://sourceforge.net/projects/freetype/files/freetype2/$($(package)_version)/
$(package)_file_name=freetype-$($(package)_version).tar.gz
$(package)_sha256_hash=5c3a8e78f7b24c20b25b54ee575d6daa40007a5f4eea2845861c3409b3021747

define $(package)_set_vars
  $(package)_config_opts := --without-zlib --without-png --without-harfbuzz --without-bzip2 --enable-static --disable-shared
  $(package)_config_opts += --enable-option-checking --without-brotli
  $(package)_config_opts += --with-pic
endef

define $(package)_preprocess_cmds
  rm -rf docs
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
  rm -rf share/man lib/*.la
endef
