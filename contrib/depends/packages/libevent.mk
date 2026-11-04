package=libevent
$(package)_version=2.1.13
$(package)_download_path=https://github.com/libevent/libevent/releases/download/release-2.1.13-stable/
$(package)_file_name=libevent-$($(package)_version)-stable.tar.gz
$(package)_sha256_hash=f7e9383b8c0baa81b687e5b5eecc01beefaf1b19b64151d95ed61647fe7a315c
$(package)_dependencies=openssl

define $(package)_set_vars
    $(package)_config_opts=--disable-shared --enable-static --with-pic
endef

define $(package)_config_cmds
    $($(package)_autoconf)
endef

define $(package)_build_cmds
    $(MAKE)
endef

define $(package)_stage_cmds
    $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef
