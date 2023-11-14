package=tor_darwin
$(package)_version=0.4.8.7
$(package)_download_path=https://dist.torproject.org/torbrowser/13.0.1/
$(package)_file_name=tor-browser-macos-13.0.1.dmg
$(package)_sha256_hash=3000b97ab595d6ce3cfd7554079c14c7fe41e5c3aef791c65c6a94aeafe526f4
$(package)_extract_cmds=mkdir -p $$($(1)_extract_dir) && \
	echo "$$($(1)_sha256_hash)  $$($(1)_source)" > $$($(1)_extract_dir)/.$$($(1)_file_name).hash && \
	$(build_SHA256SUM) -c $$($(1)_extract_dir)/.$$($(1)_file_name).hash && \
	7z x $$($(1)_source)

define $(package)_stage_cmds
    cp -a Tor\ Browser/Tor\ Browser.app/Contents/MacOS/Tor $($(package)_staging_prefix_dir)/Tor/
endef