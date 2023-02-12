package=tor_mingw32
$(package)_version=0.4.7.13
$(package)_download_path=https://dist.torproject.org/torbrowser/12.0.2/
$(package)_file_name=tor-expert-bundle-12.0.2-windows-x86_64.tar.gz
$(package)_sha256_hash=964f23d91b0a2339f655832f5787d7a3c1219f5062481c953c55cc6f302339c9

define $(package)_stage_cmds
    mkdir -p $($(package)_staging_prefix_dir)/Tor/ && \
    cp tor.exe $($(package)_staging_prefix_dir)/Tor/
endef