package=tor_mingw32
$(package)_version=0.4.9.11
$(package)_download_path=https://archive.torproject.org/tor-package-archive/torbrowser/15.0.18/
$(package)_file_name=tor-expert-bundle-windows-x86_64-15.0.18.tar.gz
$(package)_sha256_hash=6ac067402c7b4a3dc37887ed3754b3914b67fdc220c966190683e9ccf91abf0f

define $(package)_stage_cmds
    mkdir -p $($(package)_staging_prefix_dir)/Tor/ && \
    cp tor.exe $($(package)_staging_prefix_dir)/Tor/
endef
