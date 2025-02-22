package=libsquashfuse
$(package)_version=0.5.2
$(package)_download_path=https://github.com/vasi/squashfuse/releases/download/$($(package)_version)
$(package)_file_name=squashfuse-$($(package)_version).tar.gz
$(package)_sha256_hash=54e4baaa20796e86a214a1f62bab07c7c361fb7a598375576d585712691178f5
$(package)_dependencies=libfuse zstd

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
