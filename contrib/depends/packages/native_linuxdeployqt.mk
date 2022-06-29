package=native_linuxdeployqt
$(package)_version=8
$(package)_download_path=https://github.com/probonopd/linuxdeployqt/archive/refs/tags/
$(package)_file_name=8.tar.gz
$(package)_sha256_hash=5597279392431aef16997c4b6892b5aa044392f7e8869439ebea33945a8bdbad
$(package)_dependencies=qt

define $(package)_config_cmds
    qmake
endef

define $(package)_build_cmds
    $(MAKE)
endef

define $(package)_stage_cmds
    $(MAKE) INSTALL_ROOT=$($(package)_staging_dir) install
endef