package=zstd
$(package)_version=1.5.5
$(package)_download_path=https://github.com/facebook/zstd/releases/download/v$($(package)_version)
$(package)_file_name=zstd-$($(package)_version).tar.gz
$(package)_sha256_hash=9c4396cc829cfae319a6e2615202e82aad41372073482fce286fac78646d3ee4

define $(package)_build_cmds
  $($(package)_cmake) -DCMAKE_INSTALL_PREFIX=$(host_prefix) -DHOST=$(host) -DZSTD_LEGACY_SUPPORT=OFF -B build-cmake-debug -S build/cmake && \
  cd build-cmake-debug && \
  $(MAKE)
endef

define $(package)_stage_cmds
  cd build-cmake-debug && \
  $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef
