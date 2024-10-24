package=native_protobuf
$(package)_version=28.3
$(package)_download_path=https://github.com/protocolbuffers/protobuf/releases/download/v$($(package)_version)
$(package)_file_name=protobuf-$($(package)_version).tar.gz
$(package)_sha256_hash=7c3ebd7aaedd86fa5dc479a0fda803f602caaf78d8aff7ce83b89e1b8ae7442a
$(package)_dependencies=native_abseil

define $(package)_set_vars
  $(package)_cxxflags+=-std=c++17
  $(package)_config_opts=-Dprotobuf_BUILD_TESTS=OFF
  $(package)_config_opts+=-Dprotobuf_ABSL_PROVIDER=package
  $(package)_config_opts+=-Dprotobuf_BUILD_SHARED_LIBS=OFF
  $(package)_config_opts+=-Dprotobuf_WITH_ZLIB=OFF
endef

define $(package)_preprocess_cmds
  rm -rf examples docs php/src/GPBMetadata compatibility objectivec/Tests csharp/keys php/tests src/google/protobuf/testdata csharp/src/Google.Protobuf.Test
endef

define $(package)_config_cmds
  $($(package)_cmake)
endef

define $(package)_build_cmds
  $(MAKE)
endef

define $(package)_stage_cmds
  $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef

define $(package)_postprocess_cmds
  rm -rf lib64
endef
