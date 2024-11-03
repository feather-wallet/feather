package := wayland
$(package)_version := 1.23.1
$(package)_download_path := https://gitlab.freedesktop.org/wayland/wayland/-/releases/$($(package)_version)/downloads/
$(package)_file_name := wayland-$($(package)_version).tar.xz
$(package)_sha256_hash := 864fb2a8399e2d0ec39d56e9d9b753c093775beadc6022ce81f441929a81e5ed
$(package)_dependencies=native_wayland libffi expat
$(package)_patches = toolchain.txt

define $(package)_preprocess_cmds
  cp $($(package)_patch_dir)/toolchain.txt toolchain.txt && \
  sed -i -e 's|@host_prefix@|$(host_prefix)|' \
         -e 's|@build_prefix@|$(build_prefix)|' \
         -e 's|@cc@|$($(package)_cc)|' \
         -e 's|@cxx@|$($(package)_cxx)|' \
         -e 's|@ar@|$($(package)_ar)|' \
         -e 's|@strip@|$(host_STRIP)|' \
         -e 's|@arch@|$(host_arch)|' \
	  toolchain.txt
endef

define $(package)_config_cmds
  PKG_CONFIG_LIBDIR="$(host_prefix)/lib/pkgconfig:$(build_prefix)/lib/pkgconfig" PKG_CONFIG_PATH="$(build_prefix)/lib/pkgconfig" meson setup --cross-file toolchain.txt build -Ddtd_validation=false -Ddocumentation=false -Dprefer_static=true -Ddefault_library=static -Dtests=false -Dscanner=false
endef

define $(package)_build_cmds
  ninja -C build
endef

define $(package)_stage_cmds
  DESTDIR=$($(package)_staging_dir) ninja -C build install
endef
