# Needed for libappimage
package=liblzma
$(package)_version=5.2.3
$(package)_download_path=https://netcologne.dl.sourceforge.net/project/lzmautils/
$(package)_file_name=xz-$($(package)_version).tar.gz
$(package)_sha256_hash=71928b357d0a09a12a4b4c5fafca8c31c19b0e7d3b8ebb19622e96f26dbf28cb

define $(package)_set_vars
  $(package)_config_opts=--with-pic --disable-shared --enable-static --disable-xz --disable-xzdec
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
