package=tor_darwin
$(package)_version=0.4.8.19
$(package)_download_path=https://archive.torproject.org/tor-package-archive/torbrowser/15.0/
$(package)_file_name=tor-browser-macos-15.0.dmg
$(package)_sha256_hash=d297c9b5e07496020051346383a4053ddf4da5ba426e41ac48c2e1e3e870e819
$(package)_extract_cmds=mkdir -p $$($(1)_extract_dir) && \
	echo "$$($(1)_sha256_hash)  $$($(1)_source)" > $$($(1)_extract_dir)/.$$($(1)_file_name).hash && \
	$(build_SHA256SUM) -c $$($(1)_extract_dir)/.$$($(1)_file_name).hash && \
	7z x $$($(1)_source)

define $(package)_stage_cmds
    cp -a Tor\ Browser/Tor\ Browser.app/Contents/MacOS/Tor $($(package)_staging_prefix_dir)/Tor/
endef
