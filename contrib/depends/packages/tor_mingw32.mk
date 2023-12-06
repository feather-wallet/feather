package=tor_mingw32
$(package)_version=0.4.8.9
$(package)_download_path=https://dist.torproject.org/torbrowser/13.0.6/
$(package)_file_name=tor-expert-bundle-windows-x86_64-13.0.6.tar.gz
$(package)_sha256_hash=e3b885da9a53c3c734ddf1683deb520d545f899748d8f53f143d3a545a2ef1e9

define $(package)_stage_cmds
    mkdir -p $($(package)_staging_prefix_dir)/Tor/ && \
    cp tor.exe $($(package)_staging_prefix_dir)/Tor/
endef