package=zxing-cpp
$(package)_version=3.1.0
$(package)_download_path=https://github.com/zxing-cpp/zxing-cpp/releases/download/v$($(package)_version)
$(package)_file_name=$(package)-$($(package)_version).tar.gz
$(package)_sha256_hash=a3eb825154f05242283e7d94d8ebdcf95beb3a534eba393cce504e91c9b215bd

define $(package)_set_vars
  $(package)_config_opts += -DZXING_WRITERS=OFF
  $(package)_config_opts += -DZXING_EXAMPLES=OFF
  $(package)_config_opts += -DZXING_C_API=OFF
  $(package)_config_opts += -DZXING_EXAMPLES_QT=OFF
  $(package)_config_opts += -DZXING_BLACKBOX_TESTS=OFF
  $(package)_config_opts += -DBUILD_SHARED_LIBS=OFF
  $(package)_config_opts += -DZXING_TEST_DOTNET=OFF
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
