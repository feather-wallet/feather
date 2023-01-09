package=libappimage
$(package)_version=v0.1.x-legacy
$(package)_download_path=https://github.com/AppImageCommunity/libappimage/archive/refs/heads/
$(package)_file_name=$($(package)_version).tar.gz
$(package)_sha256_hash=fef3962bfb75f986f24c530a6230e95b8c79e46da3dd581543f1b615d45e7389
$(package)_dependencies=liblzma libfuse libarchive
$(package)_patches=no-unneeded-deps.patch

define $(package)_preprocess_cmds
    patch -p1 < $($(package)_patch_dir)/no-unneeded-deps.patch
endef

define $(package)_config_cmds
  $($(package)_cmake) -DCMAKE_INSTALL_PREFIX=$(host_prefix) -DCMAKE_C_COMPILER= -DUSE_SYSTEM_XZ=ON -DUSE_SYSTEM_SQUASHFUSE=ON -DUSE_SYSTEM_LIBARCHIVE=ON -DBUILD_TESTING=OFF .
endef

define $(package)_build_cmds
  $(MAKE)
endef

define $(package)_stage_cmds
  $(MAKE) DESTDIR=$($(package)_staging_dir) install && \
  cp src/libappimage_hashlib/include/hashlib.h $($(package)_staging_prefix_dir)/include/ && \
  cp src/libappimage_hashlib/include/md5.h $($(package)_staging_prefix_dir)/include/
endef
