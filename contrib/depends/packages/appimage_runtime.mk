package=appimage_runtime
$(package)_version=c9553b05938b22849ac3255ac923bf8e775ce539
$(package)_download_path=https://github.com/AppImage/type2-runtime/archive/
$(package)_file_name=$($(package)_version).tar.gz
$(package)_sha256_hash=4a27451013b571cf9f5a13660719d091cc79f2344aafa2e48578ddc0e4618af1
$(package)_dependencies=libsquashfuse zstd
$(package)_patches=depends-fix.patch

define $(package)_preprocess_cmds
  patch -p1 < $($(package)_patch_dir)/depends-fix.patch
endef

define $(package)_build_cmds
    cd src/runtime && \
    export host_prefix="$(host_prefix)" && \
    $(MAKE) runtime-fuse3 -e CC=$($(package)_cc) LDLAGS="$($(package)_ldflags)" && \
    "${HOST}-strip" runtime-fuse3
endef

define $(package)_stage_cmds
    cd src/runtime && \
    cp -a runtime-fuse3 $($(package)_staging_prefix_dir)/runtime
endef