package=tor_mingw32
$(package)_version=0.4.8.16
$(package)_download_path=https://dist.torproject.org/
$(package)_file_name=tor-$($(package)_version).tar.gz
$(package)_sha256_hash=6540dd377a120fb8e7d27530aa3b7ff72a0fa5b4f670fe1d64c987c1cfd390cb
$(package)_dependencies=libevent openssl zlib

define $(package)_set_vars
    $(package)_config_opts=--disable-asciidoc --disable-manpage --disable-html-manual --disable-system-torrc
    $(package)_config_opts+=--disable-module-relay --disable-lzma --disable-zstd
    $(package)_config_opts+=--with-libevent-dir=$(host_prefix) --with-openssl-dir=$(host_prefix)
    $(package)_config_opts+=--with-zlib-dir=$(host_prefix) --disable-tool-name-check --enable-fatal-warnings
    $(package)_config_opts+=--prefix=$(host_prefix)
    $(package)_config_opts_x86_64+=--enable-static-tor
    $(package)_cflags+=-O1
    $(package)_cxxflags+=-O1
    $(package)_ldflags+=$(guix_ldflags)
endef

define $(package)_preprocess_cmds
    rm -rf doc/man
endef

define $(package)_config_cmds
    $($(package)_autoconf) $($(package)_config_opts)
endef

define $(package)_build_cmds
    $(MAKE)
endef

define $(package)_stage_cmds
    $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef

define $(package)_postprocess_cmds
    $(host_toolchain)strip -s -D bin/tor.exe && \
    mkdir $($(package)_staging_prefix_dir)/Tor/ && \
    cp bin/tor.exe $($(package)_staging_prefix_dir)/Tor
endef
