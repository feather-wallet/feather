package=tor_darwin
$(package)_version=0.4.8.9
$(package)_download_path=https://dist.torproject.org/torbrowser/13.0.5/
$(package)_file_name=tor-browser-macos-13.0.5.dmg
$(package)_sha256_hash=c5c7efda8213f2647ed092d5cc19723343e3564d632bcf4139c23257fec456aa
$(package)_extract_cmds=mkdir -p $$($(1)_extract_dir) && \
	echo "$$($(1)_sha256_hash)  $$($(1)_source)" > $$($(1)_extract_dir)/.$$($(1)_file_name).hash && \
	$(build_SHA256SUM) -c $$($(1)_extract_dir)/.$$($(1)_file_name).hash && \
	7z x $$($(1)_source)

define $(package)_stage_cmds
    cp -a Tor\ Browser/Tor\ Browser.app/Contents/MacOS/Tor $($(package)_staging_prefix_dir)/Tor/
endef