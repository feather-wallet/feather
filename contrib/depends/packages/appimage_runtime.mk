# TODO: we're not actually using the sources downloaded here. Perhaps host patches in a github repo.
package=appimage_runtime
$(package)_version=13
$(package)_download_path=https://github.com/AppImage/AppImageKit/archive/refs/tags/
$(package)_file_name=$($(package)_version).tar.gz
$(package)_sha256_hash=51b837c78dd99ecc1cf3dd283f4a98a1be665b01457da0edc1ff736d12974b1a
$(package)_dependencies=native_cmake libsquashfuse libappimage liblzma
$(package)_patches=CMakeLists.txt runtime.c notify.c

define $(package)_preprocess_cmds
    cp -v $($(package)_patch_dir)/* .
endef

define $(package)_config_cmds
    $($(package)_cmake) -DCMAKE_INSTALL_PREFIX=$(host_prefix) .
endef

define $(package)_build_cmds
    $(MAKE)
endef

define $(package)_stage_cmds
    cp -a runtime $($(package)_staging_prefix_dir)/runtime-x86_64
endef
