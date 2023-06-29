package=flatstart
$(package)_version=a148fb86c30968eeb30dc6ac3384ad2a16690520
$(package)_download_path=https://github.com/tobtoht/ln-guix-store/archive/
$(package)_file_name=$($(package)_version).tar.gz
$(package)_sha256_hash=e9694fcda4f9fcb00374da7a439b881296c0ce67ea98ca44bf2830d1a364365b

define $(package)_build_cmds
  zig build-exe ln-guix-store.zig --strip -OReleaseSmall
endef

define $(package)_stage_cmds
  mkdir -p $($(package)_staging_prefix_dir)/bin && \
  cp ln-guix-store $($(package)_staging_prefix_dir)/bin/startup
endef
