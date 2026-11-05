package=tor_mingw32
$(package)_version=0.4.9.15
$(package)_download_path=https://archive.torproject.org/tor-package-archive/torbrowser/16.0/
$(package)_file_name=tor-expert-bundle-windows-x86_64-16.0.tar.gz
$(package)_sha256_hash=2ee123e9167318a3742778ff704e75f01d4780f05a14522d451fe011011077c4

define $(package)_stage_cmds
    mkdir -p $($(package)_staging_prefix_dir)/Tor/ && \
    cp tor.exe $($(package)_staging_prefix_dir)/Tor/
endef
