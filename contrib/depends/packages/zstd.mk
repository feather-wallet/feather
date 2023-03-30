package=zstd
$(package)_version=1.5.4
$(package)_download_path=https://github.com/facebook/zstd/releases/download/v$($(package)_version)
$(package)_file_name=zstd-$($(package)_version).tar.gz
$(package)_sha256_hash=0f470992aedad543126d06efab344dc5f3e171893810455787d38347343a4424

define $(package)_build_cmds
  $($(package)_cmake) -DCMAKE_INSTALL_PREFIX=$(host_prefix) -DHOST=$(host) -DZSTD_LEGACY_SUPPORT=OFF -B build-cmake-debug -S build/cmake && \
  cd build-cmake-debug && \
  $(MAKE)
endef

define $(package)_stage_cmds
  cd build-cmake-debug && \
  $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef
