package=tor
$(package)_version=0.4.7.7
$(package)_download_path=https://dist.torproject.org/
$(package)_file_name=tor-$($(package)_version).tar.gz
$(package)_sha256_hash=3e131158b52b9435d7e43d1c47ef288b96d005342cc44b8c950bb403851a5b44
$(package)_dependencies=libevent openssl zlib

define $(package)_set_vars
    $(package)_config_opts=--disable-asciidoc --disable-manpage --disable-html-manual --disable-system-torrc
    $(package)_config_opts+=--disable-module-relay --disable-lzma --disable-zstd --enable-static-tor
    $(package)_config_opts+=--with-libevent-dir=$(host_prefix) --with-openssl-dir=$(host_prefix)
    $(package)_config_opts+=--with-zlib-dir=$(host_prefix) --disable-tool-name-check --enable-fatal-warnings
    $(package)_config_opts+=--prefix=$(host_prefix)
endef

define $(package)_config_cmds
    ./configure $($(package)_config_opts)
endef

define $(package)_build_cmds
    $(MAKE)
endef

define $(package)_stage_cmds
    $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef

define $(package)_postprocess_cmds
    strip -s -D bin/tor && \
    mkdir $($(package)_staging_prefix_dir)/Tor/ && \
    cp bin/tor $($(package)_staging_prefix_dir)/Tor
endef