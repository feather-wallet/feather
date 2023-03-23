package=native_rcodesign
$(package)_version=0.22.0
$(package)_download_path=https://github.com/indygreg/apple-platform-rs/releases/download/apple-codesign/$($(package)_version)/
$(package)_file_name=apple-codesign-$($(package)_version)-x86_64-unknown-linux-musl.tar.gz
$(package)_sha256_hash=f6382c5e6e47bc4f6f02be2ad65a4fc5120b3df75aa520647abbadbae747fbcc

define $(package)_stage_cmds
    mkdir -p $($(package)_staging_prefix_dir)/bin && \
    cp rcodesign $($(package)_staging_prefix_dir)/bin
endef