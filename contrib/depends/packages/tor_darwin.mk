package=tor_darwin
$(package)_version=0.4.9.16
$(package)_download_path=https://archive.torproject.org/tor-package-archive/torbrowser/16.0.1/
$(package)_file_name=tor-browser-macos-16.0.1.dmg
$(package)_sha256_hash=f7c1f995969e6588a770d2096591ac397cd2cbb8a85b8c0c30ec206290ec2b90
$(package)_extract_cmds=mkdir -p $$($(1)_extract_dir) && \
	echo "$$($(1)_sha256_hash)  $$($(1)_source)" > $$($(1)_extract_dir)/.$$($(1)_file_name).hash && \
	$(build_SHA256SUM) -c $$($(1)_extract_dir)/.$$($(1)_file_name).hash && \
	7z x $$($(1)_source)

define $(package)_stage_cmds
    cp -a Tor\ Browser/Tor\ Browser.app/Contents/MacOS/Tor $($(package)_staging_prefix_dir)/Tor/
endef
