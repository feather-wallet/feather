package=libxkbcommon
$(package)_version=1.10.0
$(package)_download_path=https://github.com/xkbcommon/libxkbcommon/archive/refs/tags
$(package)_file_name=xkbcommon-$($(package)_version).tar.gz
$(package)_sha256_hash=0427585a4d6ca17c9fc1ac4b539bf303348e9080af70c5ea402503bc370a9631
$(package)_dependencies=libxcb
$(package)_patches=no-test-x11.patch toolchain.txt

define $(package)_preprocess_cmds
  patch -p1 < $($(package)_patch_dir)/no-test-x11.patch && \
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
  meson setup --cross-file toolchain.txt build -Dxkb-config-root=/usr/share/X11/xkb -Dxkb-config-extra-path=/app/share/X11/xkb
endef

define $(package)_build_cmds
  ninja -C build
endef

define $(package)_stage_cmds
  DESTDIR=$($(package)_staging_dir) ninja -C build install
endef
