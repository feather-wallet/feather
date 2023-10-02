package=tor_mingw32
$(package)_version=0.4.7.15
$(package)_download_path=https://dist.torproject.org/torbrowser/12.5.6/
$(package)_file_name=tor-expert-bundle-12.5.6-windows-x86_64.tar.gz
$(package)_sha256_hash=0db0f8fc6c60fa62b8159468fbb476acf0dec2f8dcf1b9fe7db3b91538334461

define $(package)_stage_cmds
    mkdir -p $($(package)_staging_prefix_dir)/Tor/ && \
    cp tor.exe $($(package)_staging_prefix_dir)/Tor/
endef