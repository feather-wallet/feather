package=native_patchelf
$(package)_version=0.14.5
$(package)_download_path=https://github.com/NixOS/patchelf/releases/download/$($(package)_version)/
$(package)_file_name=patchelf-$($(package)_version).tar.gz
$(package)_sha256_hash=113ada3f1ace08f0a7224aa8500f1fa6b08320d8f7df05ff58585286ec5faa6f

define $(package)_config_cmds
    ./configure --prefix=$(build_prefix)
endef

define $(package)_build_cmds
    $(MAKE)
endef

define $(package)_stage_cmds
    $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef