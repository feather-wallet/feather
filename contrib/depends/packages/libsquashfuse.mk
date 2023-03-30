package=libsquashfuse
$(package)_version=e51978cd6bb5c4d16fae9eee43d0b258f570bb0f
$(package)_download_path=https://github.com/vasi/squashfuse/archive/
$(package)_file_name=$($(package)_version).tar.gz
$(package)_sha256_hash=f544029ad30d8fbde4e4540c574b8cdc6d38b94df025a98d8551a9441f07d341
$(package)_dependencies=libfuse zstd

define $(package)_preprocess_cmds
  ./autogen.sh
endef

define $(package)_config_cmds
  $($(package)_autoconf) --with-zstd=$(host_prefix) --without-zlib CFLAGS=-no-pie LDFLAGS=-static
endef

define $(package)_build_cmds
  $(MAKE)
endef

define $(package)_stage_cmds
  $(MAKE) DESTDIR=$($(package)_staging_dir) install && \
  mkdir -p $($(package)_staging_prefix_dir)/lib && \
  mkdir -p $($(package)_staging_prefix_dir)/include/squashfuse && \
  cp .libs/libfuseprivate.a $($(package)_staging_prefix_dir)/lib/ && \
  cp .libs/libsquashfuse.a $($(package)_staging_prefix_dir)/lib/ && \
  cp .libs/libsquashfuse_ll.a $($(package)_staging_prefix_dir)/lib/ && \
  find . -name "*.h" -exec cp --parents '{}' $($(package)_staging_prefix_dir)/include/squashfuse  \;
endef
