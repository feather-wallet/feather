package=freetype
$(package)_version=2.13.1
$(package)_download_path=https://download.savannah.gnu.org/releases/freetype
$(package)_file_name=freetype-$($(package)_version).tar.xz
$(package)_sha256_hash=ea67e3b019b1104d1667aa274f5dc307d8cbd606b399bc32df308a77f1a564bf

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
