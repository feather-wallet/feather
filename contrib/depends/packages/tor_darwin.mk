package=tor_darwin
$(package)_version=0.4.8.16
$(package)_download_path=https://dist.torproject.org/torbrowser/14.0.9/
$(package)_file_name=tor-browser-macos-14.0.9.dmg
$(package)_sha256_hash=82a3dba862a34af1796e6a032f08c591e88cbac5831bedc83e168c1833d62506
$(package)_extract_cmds=mkdir -p $$($(1)_extract_dir) && \
	echo "$$($(1)_sha256_hash)  $$($(1)_source)" > $$($(1)_extract_dir)/.$$($(1)_file_name).hash && \
	$(build_SHA256SUM) -c $$($(1)_extract_dir)/.$$($(1)_file_name).hash && \
	7z x $$($(1)_source)

define $(package)_stage_cmds
    cp -a Tor\ Browser/Tor\ Browser.app/Contents/MacOS/Tor $($(package)_staging_prefix_dir)/Tor/
endef
