package=libarchive
$(package)_version=3.7.4
$(package)_download_path=https://github.com/libarchive/libarchive/releases/download/v$($(package)_version)/
$(package)_file_name=$(package)-$($(package)_version).tar.xz
$(package)_sha256_hash=f887755c434a736a609cbd28d87ddbfbe9d6a3bb5b703c22c02f6af80a802735

define $(package)_config_cmds
    $($(package)_autoconf)
endef

define $(package)_build_cmds
    $(MAKE)
endef

define $(package)_stage_cmds
    $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef
