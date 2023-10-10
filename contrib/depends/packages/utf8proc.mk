package=utf8proc
$(package)_version=2.8.0
$(package)_download_path=https://github.com/JuliaStrings/utf8proc/archive/refs/tags/
$(package)_download_file=v$($(package)_version).tar.gz
$(package)_file_name=$(package)-$($(package)_version).tar.gz
$(package)_sha256_hash=a0a60a79fe6f6d54e7d411facbfcc867a6e198608f2cd992490e46f04b1bcecc
$(package)_patches=force_static.patch

define $(package)_preprocess_cmds
  patch -p1 < $($(package)_patch_dir)/force_static.patch
endef

define $(package)_config_cmds
    echo "$($(package)_cmake)" && \
    mkdir build && \
    cd build && \
    $($(package)_cmake) ..
endef

define $(package)_build_cmds
    cd build && \
    $(MAKE)
endef

define $(package)_stage_cmds
    cd build && \
    $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef
