package=tor_darwin
$(package)_version=0.4.7.12
$(package)_download_path=https://dist.torproject.org/torbrowser/12.0.1/
$(package)_file_name=TorBrowser-12.0.1-macos_ALL.dmg
$(package)_sha256_hash=b4b52e1e5a2a0c4e1c68cf36dc8054fd1eb826d43f2622b56ef65e0f9f5db845
$(package)_extract_cmds=mkdir -p $$($(1)_extract_dir) && \
	echo "$$($(1)_sha256_hash)  $$($(1)_source)" > $$($(1)_extract_dir)/.$$($(1)_file_name).hash && \
	$(build_SHA256SUM) -c $$($(1)_extract_dir)/.$$($(1)_file_name).hash && \
	7z x $$($(1)_source)

define $(package)_stage_cmds
    mv Tor\ Browser.app/Contents/MacOS/Tor/tor.real Tor\ Browser.app/Contents/MacOS/Tor/tor && \
    cp -a Tor\ Browser.app/Contents/MacOS/Tor $($(package)_staging_prefix_dir)/Tor/
endef