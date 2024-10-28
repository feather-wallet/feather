package=tor_linux
$(package)_version=0.4.8.13
$(package)_download_path=https://dist.torproject.org/
$(package)_file_name=tor-$($(package)_version).tar.gz
$(package)_sha256_hash=9baf26c387a2820b3942da572146e6eb77c2bc66862af6297cd02a074e6fba28
$(package)_dependencies=libevent openssl zlib

define $(package)_set_vars
    $(package)_config_opts=--disable-asciidoc --disable-manpage --disable-html-manual --disable-system-torrc
    $(package)_config_opts+=--disable-module-relay --disable-lzma --disable-zstd
    $(package)_config_opts+=--with-libevent-dir=$(host_prefix) --with-openssl-dir=$(host_prefix)
    $(package)_config_opts+=--with-zlib-dir=$(host_prefix) --disable-tool-name-check --enable-fatal-warnings
    $(package)_config_opts+=--prefix=$(host_prefix)
    $(package)_config_opts_x86_64+=--enable-static-tor
    $(package)_cflags+=-O2
    $(package)_cxxflags+=-O2
    $(package)_ldflags+=$(guix_ldflags)
endef

define $(package)_preprocess_cmds
    rm Makefile.in aclocal.m4 ar-lib compile config.guess config.sub configure depcomp install-sh missing orconfig.h.in test-driver &&\
    rm -rf doc/man &&\
    autoreconf -fi
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
    $(host_toolchain)strip -s -D bin/tor && \
    mkdir $($(package)_staging_prefix_dir)/Tor/ && \
    cp bin/tor $($(package)_staging_prefix_dir)/Tor
endef
