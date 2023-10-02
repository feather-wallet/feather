package=tor_darwin
$(package)_version=0.4.7.15
$(package)_download_path=https://dist.torproject.org/torbrowser/12.5.6/
$(package)_file_name=TorBrowser-12.5.6-macos_ALL.dmg
$(package)_sha256_hash=8ba5b760e9ad959b74a056d614a9f618e3c783af30a6587b5a8194ec0baa4cea
$(package)_extract_cmds=mkdir -p $$($(1)_extract_dir) && \
	echo "$$($(1)_sha256_hash)  $$($(1)_source)" > $$($(1)_extract_dir)/.$$($(1)_file_name).hash && \
	$(build_SHA256SUM) -c $$($(1)_extract_dir)/.$$($(1)_file_name).hash && \
	7z x $$($(1)_source)

define $(package)_stage_cmds
    mv Tor\ Browser.app/Contents/MacOS/Tor/tor.real Tor\ Browser.app/Contents/MacOS/Tor/tor && \
    cp -a Tor\ Browser.app/Contents/MacOS/Tor $($(package)_staging_prefix_dir)/Tor/
endef