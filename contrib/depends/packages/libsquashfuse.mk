# Needed for libappimage
package=libsquashfuse
$(package)_version=1f980303b89c779eabfd0a0fdd36d6a7a311bf92
$(package)_download_path=https://github.com/vasi/squashfuse/archive/
$(package)_file_name=$($(package)_version).tar.gz
$(package)_sha256_hash=8cef1539bd9c9efd3c407004fdd7a3bbef44102a5966b892819a275d609013a3
$(package)_dependencies=liblzma libfuse zlib
$(package)_patches=squashfuse.patch squashfuse_dlopen.patch squashfuse_dlopen.c squashfuse_dlopen.h

# for some reason, a first run may fail, but it seems just running it a second time fixes the issues
define $(package)_preprocess_cmds
  patch -p1 < $($(package)_patch_dir)/squashfuse.patch && \
  patch -p1 < $($(package)_patch_dir)/squashfuse_dlopen.patch && \
  cp -v $($(package)_patch_dir)/squashfuse_dlopen.c $($(package)_patch_dir)/squashfuse_dlopen.h . && \
  libtoolize --force && \
  ./autogen.sh  || true && \
  ./autogen.sh && \
  sed -i "/PKG_CHECK_MODULES.*/,/,:./d" configure && \
  sed -i "s/typedef off_t sqfs_off_t/typedef int64_t sqfs_off_t/g" common.h
endef

define $(package)_set_vars
  $(package)_config_opts=--disable-demo --disable-high-level --without-lzo --without-lz4
  $(package)_config_opts+=--prefix=$(host_prefix)
  $(package)_config_opts+=--libdir=$(host_prefix)/lib
endef

define $(package)_config_cmds
  $($(package)_autoconf) $($(package)_config_opts)
endef

define $(package)_build_cmds
  $(MAKE) && \
  ls .libs
endef

define $(package)_stage_cmds
  $(MAKE) DESTDIR=$($(package)_staging_dir) install && \
  mkdir -p $($(package)_staging_prefix_dir)/lib && \
  mkdir -p $($(package)_staging_prefix_dir)/include && \
  cp .libs/libfuseprivate.a $($(package)_staging_prefix_dir)/lib/ && \
  cp .libs/libsquashfuse.a $($(package)_staging_prefix_dir)/lib/ && \
  cp .libs/libsquashfuse_ll.a $($(package)_staging_prefix_dir)/lib/ && \
  find . -name "*.h" -exec cp --parents '{}' $($(package)_staging_prefix_dir)/include/  \;
endef
