package=tor-win
$(package)_version=0.4.7.8
$(package)_download_path=https://dist.torproject.org/torbrowser/11.5.1/
$(package)_file_name=tor-win64-$($(package)_version).zip
$(package)_sha256_hash=6658aaf7d22052861631917590e248fdf8b3fe40a795d740811362b517113a47
$(package)_extract_cmds=mkdir -p $$($(1)_extract_dir) && \
	echo "$$($(1)_sha256_hash)  $$($(1)_source)" > $$($(1)_extract_dir)/.$$($(1)_file_name).hash && \
	$(build_SHA256SUM) -c $$($(1)_extract_dir)/.$$($(1)_file_name).hash && \
	unzip $$($(1)_source)

define $(package)_stage_cmds
    cp -a Tor $($(package)_staging_prefix_dir)/Tor/
endef