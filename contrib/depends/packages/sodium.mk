package=sodium
$(package)_version=1.0.19
$(package)_download_path=https://download.libsodium.org/libsodium/releases/
$(package)_file_name=libsodium-$($(package)_version).tar.gz
$(package)_sha256_hash=018d79fe0a045cca07331d37bd0cb57b2e838c51bc48fd837a1472e50068bbea

define $(package)_set_vars
  $(package)_config_opts=--enable-static --disable-shared
  $(package)_config_opts+=--prefix=$(host_prefix)
endef

define $(package)_preprocess_cmds
  cp -f $(BASEDIR)/config.guess $(BASEDIR)/config.sub build-aux/ && \
  autoconf
endef

define $(package)_config_cmds
  $($(package)_autoconf) AR_FLAGS=$($(package)_arflags)
endef

define $(package)_build_cmds
  $(MAKE)
endef

define $(package)_stage_cmds
  $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef

define $(package)_postprocess_cmds
  rm lib/*.la
endef
