package=native_abseil
$(package)_version=20250127.1
$(package)_download_path=https://github.com/abseil/abseil-cpp/archive/refs/tags/
$(package)_download_file=$($(package)_version).tar.gz
$(package)_file_name=abseil-$($(package)_version).tar.gz
$(package)_sha256_hash=b396401fd29e2e679cace77867481d388c807671dc2acc602a0259eeb79b7811

define $(package)_config_cmds
  $($(package)_cmake)
endef

define $(package)_preprocess_cmds
   rm -rf absl/time/internal/cctz/testdata
endef

define $(package)_build_cmds
  $(MAKE)
endef

define $(package)_stage_cmds
  $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef
