package=tor_mingw32
$(package)_version=0.4.8.10
$(package)_download_path=https://dist.torproject.org/torbrowser/13.0.11/
$(package)_file_name=tor-expert-bundle-windows-x86_64-13.0.11.tar.gz
$(package)_sha256_hash=78d9529b0a206e6727093ede340fc34dab2de4ea03046ff5d5d590879dc21c6e

define $(package)_stage_cmds
    mkdir -p $($(package)_staging_prefix_dir)/Tor/ && \
    cp tor.exe $($(package)_staging_prefix_dir)/Tor/
endef
