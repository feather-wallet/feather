package=native_wayland
$(package)_version=1.23.1
$(package)_download_path := https://gitlab.freedesktop.org/wayland/wayland/-/releases/$($(package)_version)/downloads/
$(package)_file_name := wayland-$($(package)_version).tar.xz
$(package)_sha256_hash := 864fb2a8399e2d0ec39d56e9d9b753c093775beadc6022ce81f441929a81e5ed
$(package)_dependencies := native_expat native_libffi

define $(package)_config_cmds
  meson setup build -Dprefix="$(build_prefix)" -Ddtd_validation=false -Ddocumentation=false -Dtests=false
endef

define $(package)_build_cmds
  ninja -C build
endef

define $(package)_stage_cmds
  DESTDIR=$($(package)_staging_dir) ninja -C build install
endef
