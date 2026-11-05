package=tor_darwin
$(package)_version=0.4.9.15
$(package)_download_path=https://archive.torproject.org/tor-package-archive/torbrowser/16.0/
$(package)_file_name=tor-browser-macos-16.0.dmg
$(package)_sha256_hash=b482caf66f2e2435b27c1714820991c3f5588653a5ca17f500c9cef99a7ba799
$(package)_extract_cmds=mkdir -p $$($(1)_extract_dir) && \
	echo "$$($(1)_sha256_hash)  $$($(1)_source)" > $$($(1)_extract_dir)/.$$($(1)_file_name).hash && \
	$(build_SHA256SUM) -c $$($(1)_extract_dir)/.$$($(1)_file_name).hash && \
	7z x $$($(1)_source)

define $(package)_stage_cmds
    cp -a Tor\ Browser/Tor\ Browser.app/Contents/MacOS/Tor $($(package)_staging_prefix_dir)/Tor/
endef
