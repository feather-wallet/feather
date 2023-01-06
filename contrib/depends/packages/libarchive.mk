# Needed for libappimage
package=libarchive
$(package)_version=3.3.1
$(package)_download_path=https://www.libarchive.org/downloads/
$(package)_file_name=libarchive-$($(package)_version).tar.gz
$(package)_sha256_hash=29ca5bd1624ca5a007aa57e16080262ab4379dbf8797f5c52f7ea74a3b0424e7

define $(package)_set_vars
  $(package)_config_opts=--with-pic --disable-shared --enable-static --disable-bsdtar --disable-bsdcat
  $(package)_config_opts+=--disable-bsdcpio --with-zlib --without-bz2lib --without-iconv --without-lz4 --without-lzma
  $(package)_config_opts+=--without-lzo2 --without-nettle --without-openssl --without-xml2 --without-expat
  $(package)_config_opts+=--prefix=$(host_prefix)
  $(package)_config_opts+=--libdir=$(host_prefix)/lib
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
