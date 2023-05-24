package=freetype
$(package)_version=2.13.0
$(package)_download_path=https://download.savannah.gnu.org/releases/freetype
$(package)_file_name=freetype-$($(package)_version).tar.xz
$(package)_sha256_hash=5ee23abd047636c24b2d43c6625dcafc66661d1aca64dec9e0d05df29592624c

define $(package)_set_vars
  $(package)_config_opts  = --without-zlib --without-png --without-harfbuzz --without-bzip2 --enable-static --disable-shared
  $(package)_config_opts += --enable-option-checking --without-brotli
  $(package)_config_opts += --with-pic
endef

define $(package)_config_cmds
  printenv && \
  echo "$($(package)_autoconf)" && \
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
