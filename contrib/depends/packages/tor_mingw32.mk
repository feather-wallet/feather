package=tor_mingw32
$(package)_version=0.4.7.12
$(package)_download_path=https://dist.torproject.org/torbrowser/12.0.1/
$(package)_file_name=tor-expert-bundle-12.0.1-windows-x86_64.tar.gz
$(package)_sha256_hash=f53cfbc4f4454a265f10a853219b40067ef73feffc4a7d67472b61dbab9129ba

define $(package)_stage_cmds
    mkdir -p $($(package)_staging_prefix_dir)/Tor/ && \
    cp tor.exe $($(package)_staging_prefix_dir)/Tor/
endef