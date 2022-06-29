package=tor-win
$(package)_version=0.4.7.7
$(package)_download_path=https://dist.torproject.org/torbrowser/11.0.13/
$(package)_file_name=tor-win64-$($(package)_version).zip
$(package)_sha256_hash=ba2a8610e13656262bc4b0da33e814f0eff3a32d4e255d4a65a77cd78ed2b3e5
$(package)_extract_cmds=mkdir -p $$($(1)_extract_dir) && \
	echo "$$($(1)_sha256_hash)  $$($(1)_source)" > $$($(1)_extract_dir)/.$$($(1)_file_name).hash && \
	$(build_SHA256SUM) -c $$($(1)_extract_dir)/.$$($(1)_file_name).hash && \
	unzip $$($(1)_source)

define $(package)_stage_cmds
    cp -a Tor $($(package)_staging_prefix_dir)/Tor/
endef