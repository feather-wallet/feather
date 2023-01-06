# Needed for libsquashfuse
package=libfuse
$(package)_version=2.9.9
$(package)_download_path=https://github.com/libfuse/libfuse/releases/download/fuse-2.9.9/
$(package)_file_name=fuse-$($(package)_version).tar.gz
$(package)_sha256_hash=d0e69d5d608cc22ff4843791ad097f554dd32540ddc9bed7638cc6fea7c1b4b5
$(package)_patches = arm64.patch

define $(package)_config_cmds
  $($(package)_autoconf)
endef

define $(package)_preprocess_cmds
  patch -p1 -i $($(package)_patch_dir)/arm64.patch
endef

define $(package)_build_cmds
  $(MAKE)
endef

define $(package)_stage_cmds
  $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef
