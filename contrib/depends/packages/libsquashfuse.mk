package=libsquashfuse
$(package)_version=0.6.1
$(package)_download_path=https://github.com/vasi/squashfuse/releases/download/$($(package)_version)
$(package)_file_name=squashfuse-$($(package)_version).tar.gz
$(package)_sha256_hash=7b18a58c40a3161b5c329ae925b72336b5316941f906b446b8ed6c5a90989f8c
$(package)_dependencies=libfuse zstd

define $(package)_config_cmds
  $($(package)_autoconf) --with-zstd=$(host_prefix) --without-zlib CFLAGS="-no-pie -Wno-error=incompatible-pointer-types" LDFLAGS=-static
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
