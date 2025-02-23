package=flatstart
$(package)_version=a148fb86c30968eeb30dc6ac3384ad2a16690520
$(package)_download_path=https://github.com/tobtoht/ln-guix-store/archive/
$(package)_file_name=$($(package)_version).tar.gz
$(package)_sha256_hash=e9694fcda4f9fcb00374da7a439b881296c0ce67ea98ca44bf2830d1a364365b
$(package)_patches=main.c main.S

define $(package)_preprocess_cmds
  cp -f $($(package)_patch_dir)/main.c $($(package)_patch_dir)/main.S .
endef

define $(package)_build_cmds
  $($(package)_cc) -nostdlib -fno-unwind-tables -fno-asynchronous-unwind-tables -fdata-sections -Wl,--gc-sections -Wa,--noexecstack -fno-builtin -fno-stack-protector -static -o startup main.c main.S
endef

define $(package)_stage_cmds
  mkdir -p $($(package)_staging_prefix_dir)/bin && \
  cp startup $($(package)_staging_prefix_dir)/bin/
endef
