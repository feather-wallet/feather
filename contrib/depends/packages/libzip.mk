package=libzip
$(package)_version=1.11.3
$(package)_download_path=https://libzip.org/download/
$(package)_file_name=$(package)-$($(package)_version).tar.gz
$(package)_sha256_hash=76653f135dde3036036c500e11861648ffbf9e1fc5b233ff473c60897d9db0ea
$(package)_dependencies=zlib

define $(package)_set_vars
  $(package)_config_opts := -DENABLE_BZIP2=OFF
  $(package)_config_opts += -DENABLE_LZMA=OFF
  $(package)_config_opts += -DENABLE_ZSTD=OFF
  $(package)_config_opts += -DENABLE_COMMONCRYPTO=OFF
  $(package)_config_opts += -DENABLE_GNUTLS=OFF
  $(package)_config_opts += -DENABLE_MBEDTLS=OFF
  $(package)_config_opts += -DENABLE_OPENSSL=OFF
  $(package)_config_opts += -DENABLE_WINDOWS_CRYPTO=OFF
  $(package)_config_opts += -DBUILD_SHARED_LIBS=OFF
  $(package)_config_opts += -DBUILD_TOOLS=OFF
  $(package)_config_opts += -DBUILD_REGRESS=OFF
  $(package)_config_opts += -DBUILD_OSSFUZZ=OFF
  $(package)_config_opts += -DBUILD_EXAMPLES=OFF
  $(package)_config_opts += -DBUILD_DOC=OFF
endef

define $(package)_preprocess_cmds
    rm -rf regress ossfuzz
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
