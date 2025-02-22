package=appimage_runtime
$(package)_version=f6c0b18d42eb4aa629cc6a18d08c20cc67c80af8
$(package)_download_path=https://github.com/AppImage/type2-runtime/archive/
$(package)_file_name=$($(package)_version).tar.gz
$(package)_sha256_hash=746c2110049c4092dab9d8b788e2a9e98ec77ff9b8b09be844fce1dcdeb4c328
$(package)_dependencies=libsquashfuse zstd
$(package)_patches=depends-fix.patch

define $(package)_preprocess_cmds
  patch -p1 < $($(package)_patch_dir)/depends-fix.patch
endef

define $(package)_build_cmds
    cd src/runtime && \
    export host_prefix="$(host_prefix)" && \
    $(MAKE) runtime -e CC=$($(package)_cc) LDLAGS="$($(package)_ldflags)" && \
    "${HOST}-strip" runtime
endef

define $(package)_stage_cmds
    cd src/runtime && \
    cp -a runtime $($(package)_staging_prefix_dir)/runtime
endef
