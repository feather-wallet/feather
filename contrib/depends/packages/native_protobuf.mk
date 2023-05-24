package=native_protobuf
$(package)_version=3.20.3
$(package)_download_path=https://github.com/protocolbuffers/protobuf/releases/download/v$($(package)_version)/
$(package)_file_name=protobuf-cpp-$($(package)_version).tar.gz
$(package)_sha256_hash=e51cc8fc496f893e2a48beb417730ab6cbcb251142ad8b2cd1951faa5c76fe3d
$(package)_cxxflags=-std=c++11

define $(package)_set_vars
  $(package)_config_opts=--disable-shared --prefix=$(build_prefix)
  $(package)_config_opts_linux=--with-pic
endef

define $(package)_config_cmds
  $($(package)_autoconf)
endef

define $(package)_build_cmds
  $(MAKE) -C src
endef

define $(package)_stage_cmds
  $(MAKE) DESTDIR=$($(package)_staging_dir) -C src install
endef

define $(package)_postprocess_cmds
  rm lib/libprotoc.a
endef
