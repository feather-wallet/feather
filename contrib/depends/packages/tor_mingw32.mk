package=tor_mingw32
$(package)_version=0.4.8.16
$(package)_download_path=https://dist.torproject.org/torbrowser/14.0.9/
$(package)_file_name=tor-expert-bundle-windows-x86_64-14.0.9.tar.gz
$(package)_sha256_hash=dce4dfd488ed8220b97c613f08216b63c6895bbefd0d091a3156501f188fe5f4

define $(package)_stage_cmds
    mkdir -p $($(package)_staging_prefix_dir)/Tor/ && \
    cp tor.exe $($(package)_staging_prefix_dir)/Tor/
endef
