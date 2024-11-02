package=darwin_sdk
$(package)_version=15.0
$(package)_download_path=https://bitcoincore.org/depends-sources/sdks
$(package)_file_name=Xcode-15.0-15A240d-extracted-SDK-with-libcxx-headers.tar.gz
$(package)_sha256_hash=c0c2e7bb92c1fee0c4e9f3a485e4530786732d6c6dd9e9f418c282aa6892f55d

define $(package)_stage_cmds
  mkdir -p $($(package)_staging_dir)/$(host_prefix)/native/SDK &&\
  mv * $($(package)_staging_dir)/$(host_prefix)/native/SDK
endef
