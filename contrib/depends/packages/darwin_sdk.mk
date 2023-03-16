package=darwin_sdk
$(package)_version=12.2
$(package)_download_path=https://featherwallet.org/files/sources/
$(package)_file_name=Xcode-12.2-12B45b-extracted-SDK-with-libcxx-headers.tar.gz
$(package)_sha256_hash=332477876917786b26dd7c3fc1665d2c5cdca81c72755e6a9754f308de77d33b

define $(package)_stage_cmds
  mkdir -p $($(package)_staging_dir)/$(host_prefix)/native/SDK &&\
  mv * $($(package)_staging_dir)/$(host_prefix)/native/SDK
endef
