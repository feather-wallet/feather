package=appimage_runtime
$(package)_version=c1ea7509bc179a05d907baca64f41875662f35f2
$(package)_download_path=https://github.com/AppImage/type2-runtime/archive/
$(package)_file_name=$($(package)_version).tar.gz
$(package)_sha256_hash=a7906c7d1610eacb6c67a16c554fac8875b05424fea49e291311c3e5db2237a3
$(package)_dependencies=libsquashfuse zstd
$(package)_patches=depends-fix.patch

define $(package)_preprocess_cmds
  patch -p1 < $($(package)_patch_dir)/depends-fix.patch
endef

define $(package)_build_cmds
    cd src/runtime && \
    export host_prefix="$(host_prefix)" && \
    $(MAKE) runtime-fuse2 -e CC=$($(package)_cc) LDLAGS="$($(package)_ldflags)" && \
    "${HOST}-strip" runtime-fuse2 && \
    echo -ne 'AI\x02' | dd of=runtime-fuse2 bs=1 count=3 seek=8 conv=notrunc
endef

define $(package)_stage_cmds
    cd src/runtime && \
    cp -a runtime-fuse2 $($(package)_staging_prefix_dir)/runtime
endef