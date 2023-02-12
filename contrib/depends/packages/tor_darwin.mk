package=tor_darwin
$(package)_version=0.4.7.13
$(package)_download_path=https://dist.torproject.org/torbrowser/12.0.2/
$(package)_file_name=TorBrowser-12.0.2-macos_ALL.dmg
$(package)_sha256_hash=c6968a7041890de6ac344e9163ce0a1c0fb82394a2da9d4e7f77e0bce7a1c952
$(package)_extract_cmds=mkdir -p $$($(1)_extract_dir) && \
	echo "$$($(1)_sha256_hash)  $$($(1)_source)" > $$($(1)_extract_dir)/.$$($(1)_file_name).hash && \
	$(build_SHA256SUM) -c $$($(1)_extract_dir)/.$$($(1)_file_name).hash && \
	7z x $$($(1)_source)

define $(package)_stage_cmds
    mv Tor\ Browser.app/Contents/MacOS/Tor/tor.real Tor\ Browser.app/Contents/MacOS/Tor/tor && \
    cp -a Tor\ Browser.app/Contents/MacOS/Tor $($(package)_staging_prefix_dir)/Tor/
endef