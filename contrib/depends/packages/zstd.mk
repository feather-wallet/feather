package=zstd
$(package)_version=1.5.7
$(package)_download_path=https://github.com/facebook/zstd/releases/download/v$($(package)_version)
$(package)_file_name=zstd-$($(package)_version).tar.gz
$(package)_sha256_hash=eb33e51f49a15e023950cd7825ca74a4a2b43db8354825ac24fc1b7ee09e6fa3

define $(package)_preprocess_cmds
  rm -rf tests doc contrib/pzstd/images build/single_file_libs/examples
endef

define $(package)_build_cmds
  $($(package)_cmake) -DCMAKE_INSTALL_PREFIX=$(host_prefix) -DHOST=$(host) -DZSTD_LEGACY_SUPPORT=OFF -B build-cmake-debug -S build/cmake && \
  cd build-cmake-debug && \
  $(MAKE)
endef

define $(package)_stage_cmds
  cd build-cmake-debug && \
  $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef
