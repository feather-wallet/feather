package=unbound
$(package)_version=1.23.1
$(package)_download_path=https://www.nlnetlabs.nl/downloads/$(package)/
$(package)_file_name=$(package)-$($(package)_version).tar.gz
$(package)_sha256_hash=6a6b117c799d8de3868643397e0fd71591f6d42f4473f598bdb22609ff362590
$(package)_dependencies=openssl expat

define $(package)_set_vars
  $(package)_config_opts=--disable-shared --enable-static --without-pyunbound --prefix=$(host_prefix)
  $(package)_config_opts+=--with-libexpat=$(host_prefix) --with-ssl=$(host_prefix) --with-libevent=no
  $(package)_config_opts+=--without-pythonmodule --disable-flto --with-pthreads --with-libunbound-only
  $(package)_config_opts_linux=--with-pic
  $(package)_config_opts_w64=--enable-static-exe --sysconfdir=/etc --prefix=$(host_prefix) --target=$(host_prefix)
  $(package)_config_opts_x86_64_darwin=ac_cv_func_SHA384_Init=yes
  $(package)_build_opts_mingw32=LDFLAGS="$($(package)_ldflags) -lpthread"
endef

define $(package)_preprocess_cmds
  rm doc/*.odp doc/*.pdf contrib/*.tar.gz contrib/*.tar.bz2 &&\
  rm -rf testdata dnscrypt/testdata
endef

define $(package)_config_cmds
  $($(package)_autoconf) ac_cv_func_getentropy=no
endef

define $(package)_build_cmds
  $(MAKE) $($(package)_build_opts)
endef

define $(package)_stage_cmds
  $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef

define $(package)_postprocess_cmds
  rm -rf share
endef
