package=libfuse
$(package)_version=3.16.2
$(package)_download_path=https://github.com/libfuse/libfuse/releases/download/fuse-$($(package)_version)
$(package)_file_name=fuse-$($(package)_version).tar.gz
$(package)_sha256_hash=f797055d9296b275e981f5f62d4e32e089614fc253d1ef2985851025b8a0ce87
$(package)_patches=toolchain.txt mount.c.diff no-dlopen.patch set_fusermount_path.patch

define $(package)_preprocess_cmds
  patch -p1 < $($(package)_patch_dir)/set_fusermount_path.patch && \
  patch -p1 < $($(package)_patch_dir)/no-dlopen.patch && \
  patch -p1 < $($(package)_patch_dir)/mount.c.diff && \
  cp $($(package)_patch_dir)/toolchain.txt toolchain.txt && \
  sed -i -e 's|@host_prefix@|$(host_prefix)|' \
         -e 's|@cc@|$($(package)_cc)|' \
         -e 's|@cxx@|$($(package)_cxx)|' \
         -e 's|@ar@|$($(package)_ar)|' \
         -e 's|@strip@|$(host_STRIP)|' \
         -e 's|@arch@|$(host_arch)|' \
	  toolchain.txt
endef

define $(package)_config_cmds
  meson setup --cross-file toolchain.txt build
endef

define $(package)_build_cmds
  ninja -C build
endef

define $(package)_stage_cmds
  DESTDIR=$($(package)_staging_dir) ninja -C build install
endef
