package=utf8proc
$(package)_version=2.9.0
$(package)_download_path=https://github.com/JuliaStrings/utf8proc/releases/download/v$($(package)_version)/
$(package)_file_name=$(package)-$($(package)_version).tar.gz
$(package)_sha256_hash=bd215d04313b5bc42c1abedbcb0a6574667e31acee1085543a232204e36384c4
$(package)_patches=force_static.patch
$(package)_build_subdir=build

define $(package)_preprocess_cmds
  patch -p1 < $($(package)_patch_dir)/force_static.patch
endef

define $(package)_config_cmds
  $($(package)_cmake) -S .. -B .
endef

define $(package)_build_cmds
  $(MAKE)
endef

define $(package)_stage_cmds
  $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef
