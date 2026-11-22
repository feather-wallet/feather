package=tor_mingw32
$(package)_version=0.4.9.16
$(package)_download_path=https://archive.torproject.org/tor-package-archive/torbrowser/16.0.1/
$(package)_file_name=tor-expert-bundle-windows-x86_64-16.0.1.tar.gz
$(package)_sha256_hash=9eed3c3ff1fe43df034ce360196c9ec95472560d77ff55614d0cd5120edea27b

define $(package)_stage_cmds
    mkdir -p $($(package)_staging_prefix_dir)/Tor/ && \
    cp tor.exe $($(package)_staging_prefix_dir)/Tor/
endef
