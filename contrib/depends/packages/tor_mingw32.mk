package=tor_mingw32
$(package)_version=0.4.8.19
$(package)_download_path=https://archive.torproject.org/tor-package-archive/torbrowser/15.0/
$(package)_file_name=tor-expert-bundle-windows-x86_64-15.0.tar.gz
$(package)_sha256_hash=3cf5c94d0538da8c992476063cea0ed0a5afb534c0b69580f6d72a78a2a2d51c

define $(package)_stage_cmds
    mkdir -p $($(package)_staging_prefix_dir)/Tor/ && \
    cp tor.exe $($(package)_staging_prefix_dir)/Tor/
endef
