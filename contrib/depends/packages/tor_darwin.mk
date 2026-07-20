package=tor_darwin
$(package)_version=0.4.9.11
$(package)_download_path=https://archive.torproject.org/tor-package-archive/torbrowser/15.0.18/
$(package)_file_name=tor-browser-macos-15.0.18.dmg
$(package)_sha256_hash=bf3410e7c39bbf6fa85979bca95964f74063d1bc3f2a765b59065efce5f5dbc4
$(package)_extract_cmds=mkdir -p $$($(1)_extract_dir) && \
	echo "$$($(1)_sha256_hash)  $$($(1)_source)" > $$($(1)_extract_dir)/.$$($(1)_file_name).hash && \
	$(build_SHA256SUM) -c $$($(1)_extract_dir)/.$$($(1)_file_name).hash && \
	7z x $$($(1)_source)

define $(package)_stage_cmds
    cp -a Tor\ Browser/Tor\ Browser.app/Contents/MacOS/Tor $($(package)_staging_prefix_dir)/Tor/
endef
