package=libXau
$(package)_version=1.0.12
$(package)_download_path=https://xorg.freedesktop.org/releases/individual/lib/
$(package)_file_name=libXau-$($(package)_version).tar.gz
$(package)_sha256_hash=2402dd938da4d0a332349ab3d3586606175e19cb32cb9fe013c19f1dc922dcee
$(package)_dependencies=xorgproto
$(package)_patches=toolchain.txt

define $(package)_preprocess_cmds
  rm Makefile.in aclocal.m4 compile config.guess config.h.in config.sub configure depcomp \
     install-sh ltmain.sh missing test-driver man/Makefile.in && \
  rm -rf m4 && \
  cp $($(package)_patch_dir)/toolchain.txt toolchain.txt && \
  sed -i -e 's|@host_prefix@|$(host_prefix)|' \
         -e 's|@cc@|$($(package)_cc)|' \
         -e 's|@cxx@|$($(package)_cxx)|' \
         -e 's|@ar@|$($(package)_ar)|' \
         -e 's|@strip@|$(host_STRIP)|' \
         -e 's|@arch@|$(host_arch)|' \
	  toolchain.txt
endef

define $(package)_config_cmds
  meson setup --cross-file toolchain.txt build
endef

define $(package)_build_cmds
  ninja -C build
endef

define $(package)_stage_cmds
  DESTDIR=$($(package)_staging_dir) ninja -C build install
endef
