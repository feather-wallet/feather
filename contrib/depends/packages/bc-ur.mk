package=bc-ur
$(package)_version=0.3.0
$(package)_download_path=https://github.com/BlockchainCommons/$(package)/archive/refs/tags/
$(package)_download_file=$($(package)_version).tar.gz
$(package)_file_name=$(package)-$($(package)_version).tar.gz
$(package)_sha256_hash=2b9455766ce84ae9f7013c9a72d749034dddefb3f515145d585c732f17e7fa94
$(package)_patches=build-fix.patch

define $(package)_preprocess_cmds
  patch -p1 < $($(package)_patch_dir)/build-fix.patch
endef

define $(package)_config_cmds
   $($(package)_cmake) -DCMAKE_INSTALL_PREFIX=$(host_prefix) -DCMAKE_C_COMPILER= .
endef

define $(package)_build_cmds
  $(MAKE)
endef

define $(package)_stage_cmds
  $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef
