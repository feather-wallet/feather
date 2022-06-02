package=tor-win
$(package)_version=0.4.7.8
$(package)_download_path=https://dist.torproject.org/torbrowser/11.5.1/
$(package)_file_name=TorBrowser-11.5.1-osx64_en-US.dmg
$(package)_sha256_hash=616d719572e4917d1264c622033afb1b4dd98e2553a0d09fd72470c99bad48e5
$(package)_extract_cmds=mkdir -p $$($(1)_extract_dir) && \
	echo "$$($(1)_sha256_hash)  $$($(1)_source)" > $$($(1)_extract_dir)/.$$($(1)_file_name).hash && \
	$(build_SHA256SUM) -c $$($(1)_extract_dir)/.$$($(1)_file_name).hash && \
	7z x $$($(1)_source)

define $(package)_stage_cmds
    mv Tor\ Browser.app/Contents/MacOS/Tor/tor.real Tor\ Browser.app/Contents/MacOS/Tor/tor && \
    cp -a Tor\ Browser.app/Contents/MacOS/Tor $($(package)_staging_prefix_dir)/Tor/
endef