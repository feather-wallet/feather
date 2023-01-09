package=native_cmake
$(package)_version=3.23.2
$(package)_version_dot=v3.23
$(package)_download_path=https://cmake.org/files/$($(package)_version_dot)/
$(package)_file_name=cmake-$($(package)_version).tar.gz
$(package)_sha256_hash=f316b40053466f9a416adf981efda41b160ca859e97f6a484b447ea299ff26aa

define $(package)_config_cmds
  ./bootstrap --prefix=$(build_prefix) -- -DCMAKE_USE_OPENSSL=OFF
endef

define $(package)_build_cmd
  +$(MAKE)
endef

define $(package)_stage_cmds
  $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef