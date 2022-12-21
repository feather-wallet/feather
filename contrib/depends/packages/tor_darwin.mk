package=tor_darwin
$(package)_version=0.4.7.11
$(package)_download_path=https://dist.torproject.org/torbrowser/11.5.8/
$(package)_file_name=TorBrowser-11.5.8-osx64_en-US.dmg
$(package)_sha256_hash=051e5a92ff493826e569eb5487adb4f767ed4936edf7ca8f5c12a8b31e1fad16
$(package)_extract_cmds=mkdir -p $$($(1)_extract_dir) && \
	echo "$$($(1)_sha256_hash)  $$($(1)_source)" > $$($(1)_extract_dir)/.$$($(1)_file_name).hash && \
	$(build_SHA256SUM) -c $$($(1)_extract_dir)/.$$($(1)_file_name).hash && \
	7z x $$($(1)_source)

define $(package)_stage_cmds
    mv Tor\ Browser.app/Contents/MacOS/Tor/tor.real Tor\ Browser.app/Contents/MacOS/Tor/tor && \
    cp -a Tor\ Browser.app/Contents/MacOS/Tor $($(package)_staging_prefix_dir)/Tor/
endef