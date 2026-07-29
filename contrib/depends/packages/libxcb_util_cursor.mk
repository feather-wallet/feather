package=libxcb_util_cursor
$(package)_version=0.1.6
$(package)_download_path=https://xcb.freedesktop.org/dist
$(package)_file_name=xcb-util-cursor-$($(package)_version).tar.gz
$(package)_sha256_hash=eae38b2dfc5c529a886e507ef576b12d2a20aa1f149608e4853af760f31be60b
$(package)_dependencies=libxcb libxcb_util_render libxcb_util_image
$(package)_patches=flatpak.patch

define $(package)_set_vars
$(package)_config_opts := --disable-shared --disable-devel-docs --without-doxygen
$(package)_config_opts += --disable-dependency-tracking --enable-option-checking
$(package)_config_opts += --datadir=/usr/share
endef

define $(package)_preprocess_cmds
  patch -p1 -i $($(package)_patch_dir)/flatpak.patch && \
  cp -f $(BASEDIR)/config.guess $(BASEDIR)/config.sub .
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

define $(package)_postprocess_cmds
  rm -rf share/man share/doc lib/*.la
endef
