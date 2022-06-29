package=native_squashfs-tools
$(package)_version=4.5.1
$(package)_download_path=https://github.com/plougher/squashfs-tools/archive/refs/tags
$(package)_file_name=$($(package)_version).tar.gz
$(package)_sha256_hash=277b6e7f75a4a57f72191295ae62766a10d627a4f5e5f19eadfbc861378deea7

define $(package)_build_cmds
    cd squashfs-tools && \
    $(MAKE)
endef

define $(package)_stage_cmds
    cd squashfs-tools && \
    $(MAKE) INSTALL_DIR=$($(package)_staging_dir)$(build_prefix) INSTALL_MANPAGES_DIR="" install
endef