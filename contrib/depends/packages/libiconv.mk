package=libiconv
$(package)_version=1.17
$(package)_download_path=https://ftp.gnu.org/gnu/libiconv
$(package)_file_name=libiconv-$($(package)_version).tar.gz
$(package)_sha256_hash=8f74213b56238c85a50a5329f77e06198771e70dd9a739779f4c02f65d971313

define $(package)_set_vars
  $(package)_config_opts=--disable-nls
  $(package)_config_opts=--enable-static
  $(package)_config_opts=--disable-shared
  $(package)_config_opts_linux=--with-pic
  $(package)_config_opts_freebsd=--with-pic
endef

define $(package)_config_cmds
  $($(package)_autoconf) AR_FLAGS=$($(package)_arflags)
endef

define $(package)_build_cmds
  $(MAKE)
endef

define $(package)_stage_cmds
  $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef

define $(package)_postprocess_cmds
  rm lib/*.la
endef
