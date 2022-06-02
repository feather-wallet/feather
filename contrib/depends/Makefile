.NOTPARALLEL :

# Pattern rule to print variables, e.g. make print-top_srcdir
print-%: FORCE
	@echo '$*'='$($*)'

# When invoking a sub-make, keep only the command line variable definitions
# matching the pattern in the filter function.
#
# e.g. invoking:
#   $ make A=1 C=1 print-MAKEOVERRIDES print-MAKEFLAGS
#
# with the following in the Makefile:
#   MAKEOVERRIDES := $(filter A=% B=%,$(MAKEOVERRIDES))
#
# will print:
#   MAKEOVERRIDES = A=1
#   MAKEFLAGS = -- A=1
#
# this is because as the GNU make manual says:
#   The command line variable definitions really appear in the variable
#   MAKEOVERRIDES, and MAKEFLAGS contains a reference to this variable.
#
# and since the GNU make manual also says:
#   variables defined on the command line are passed to the sub-make through
#   MAKEFLAGS
#
# this means that sub-makes will be invoked as if:
#   $(MAKE) A=1 blah blah
MAKEOVERRIDES := $(filter V=%,$(MAKEOVERRIDES))
SOURCES_PATH ?= $(BASEDIR)/sources
WORK_PATH = $(BASEDIR)/work
BASE_CACHE ?= $(BASEDIR)/built
SDK_PATH ?= $(BASEDIR)/SDKs
FALLBACK_DOWNLOAD_PATH ?= https://downloads.getmonero.org/depends-sources

BUILD = $(shell ./config.guess)
HOST ?= $(BUILD)
PATCHES_PATH = $(BASEDIR)/patches
BASEDIR = $(CURDIR)
HASH_LENGTH:=11
DOWNLOAD_CONNECT_TIMEOUT:=30
DOWNLOAD_RETRIES:=3
HOST_ID_SALT ?= salt
BUILD_ID_SALT ?= salt

host:=$(BUILD)
ifneq ($(HOST),)
host:=$(HOST)
endif

ifneq ($(DEBUG),)
release_type=Debug
else
release_type=Release
endif

ifneq ($(TESTS),)
build_tests=ON
release_type=Debug
else
build_tests=OFF
endif

base_build_dir=$(WORK_PATH)/build
base_staging_dir=$(WORK_PATH)/staging
base_download_dir=$(WORK_PATH)/download
canonical_host:=$(shell ./config.sub $(HOST))
build:=$(shell ./config.sub $(BUILD))

build_arch =$(firstword $(subst -, ,$(build)))
build_vendor=$(word 2,$(subst -, ,$(build)))
full_build_os:=$(subst $(build_arch)-$(build_vendor)-,,$(build))
build_os:=$(findstring linux,$(full_build_os))
build_os+=$(findstring darwin,$(full_build_os))
build_os:=$(strip $(build_os))
ifeq ($(build_os),)
build_os=$(full_build_os)
endif

host_arch=$(firstword $(subst -, ,$(canonical_host)))
host_vendor=$(word 2,$(subst -, ,$(canonical_host)))
full_host_os:=$(subst $(host_arch)-$(host_vendor)-,,$(canonical_host))
host_os:=$(findstring android,$(full_host_os))
ifeq ($(host_os),)
host_os:=$(findstring linux,$(full_host_os))
endif
host_os+=$(findstring darwin,$(full_host_os))
host_os+=$(findstring freebsd,$(full_host_os))
host_os+=$(findstring mingw32,$(full_host_os))
host_os:=$(strip $(host_os))
ifeq ($(host_os),)
host_os=$(full_host_os)
endif

$(host_arch)_$(host_os)_prefix=$(BASEDIR)/$(host)
$(host_arch)_$(host_os)_host=$(host)
host_prefix=$($(host_arch)_$(host_os)_prefix)
build_prefix=$(host_prefix)/native
ifeq ($(host_os),mingw32)
host_cmake=Windows
endif
ifeq ($(host_os),linux)
host_cmake=Linux
endif
ifeq ($(host_os),freebsd)
host_cmake=FreeBSD
endif
ifeq ($(host_os),darwin)
host_cmake=Darwin
endif
ifeq ($(host_os),android)
host_cmake=Android
endif

AT_$(V):=
AT_:=@
AT:=$(AT_$(V))
build_host=$(build)

all: install

include hosts/$(host_os).mk
include hosts/default.mk
include builders/$(build_os).mk
include builders/default.mk
include packages/packages.mk

# Previously, we directly invoked the well-known programs using $(shell ...)
# to construct build_id_string. However, that was problematic because:
#
# 1. When invoking a shell, GNU Make special-cases exit code 127 (command not
#    found) by not capturing the output but instead passing it through. This is
#    not done for any other exit code.
#
# 2. Characters like '#' (from these programs' output) would end up in make
#    variables like build_id_string, which would be wrongly interpreted by make
#    when these variables were used.
#
# Therefore, we should avoid having arbitrary strings in make variables where
# possible. The gen_id script used here hashes the output to construct a
# "make-safe" id.
#
# Also note that these lines need to be:
#
#     1. After including {hosts,builders}/*.mk, since they rely on the tool
#        variables (e.g. build_CC, host_STRIP, etc.) to be set.
#
#     2. Before including packages/*.mk (excluding packages/packages.mk), since
#        they rely on the build_id variables
#
build_id:=$(shell env CC='$(build_CC)' C_STANDARD='$(C_STANDARD)' CXX='$(build_CXX)' CXX_STANDARD='$(CXX_STANDARD)' AR='$(build_AR)' RANLIB='$(build_RANLIB)' STRIP='$(build_STRIP)' SHA256SUM='$(build_SHA256SUM)' DEBUG='$(DEBUG)' ./gen_id '$(BUILD_ID_SALT)' 'GUIX_ENVIRONMENT=$(realpath $(GUIX_ENVIRONMENT))')
$(host_arch)_$(host_os)_id:=$(shell env CC='$(host_CC)' C_STANDARD='$(C_STANDARD)' CXX='$(host_CXX)' CXX_STANDARD='$(CXX_STANDARD)' AR='$(host_AR)' RANLIB='$(host_RANLIB)' STRIP='$(host_STRIP)' SHA256SUM='$(build_SHA256SUM)' DEBUG='$(DEBUG)' ./gen_id '$(HOST_ID_SALT)' 'GUIX_ENVIRONMENT=$(realpath $(GUIX_ENVIRONMENT))')


qt_packages_$(NO_QT) = $(qt_packages) $(qt_$(host_os)_packages) $(qt_$(host_arch)_$(host_os)_packages) $(qrencode_packages_)
tor_packages_$(NO_TOR) = $(tor_packages) $(tor_$(host_os)_packages)

packages += $($(host_arch)_$(host_os)_packages) $($(host_os)_packages) $(qt_packages_) $(tor_packages_)
native_packages += $($(host_arch)_$(host_os)_native_packages) $($(host_os)_native_packages)

all_packages = $(packages) $(native_packages)

meta_depends = Makefile funcs.mk builders/default.mk hosts/default.mk hosts/$(host_os).mk builders/$(build_os).mk

$(host_arch)_$(host_os)_native_binutils?=$($(host_os)_native_binutils)
$(host_arch)_$(host_os)_native_toolchain?=$($(host_os)_native_toolchain)

include funcs.mk

CONF_PKGS := cmake-conf mxe-conf

build-only-$(1)_$(3): CMAKE_RUNRESULT_FILE = $(PREFIX)/share/cmake/modules/TryRunResults.cmake
build-only-$(1)_$(3): CMAKE_TOOLCHAIN_FILE = $(PREFIX)/$(3)/share/cmake/mxe-conf.cmake
build-only-$(1)_$(3): CMAKE_TOOLCHAIN_DIR  = $(PREFIX)/$(3)/share/cmake/mxe-conf.d
build-only-$(1)_$(3): CMAKE_STATIC_BOOL = $(if $(findstring shared,$(3)),OFF,ON)
build-only-$(1)_$(3): CMAKE_SHARED_BOOL = $(if $(findstring shared,$(3)),ON,OFF)


toolchain_path=$($($(host_arch)_$(host_os)_native_toolchain)_prefixbin)
final_build_id_long+=$(shell $(build_SHA256SUM) config.site.in)
final_build_id_long+=$(shell $(build_SHA256SUM) toolchain.cmake.in)
final_build_id+=$(shell echo -n "$(final_build_id_long)" | $(build_SHA256SUM) | cut -c-$(HASH_LENGTH))
$(host_prefix)/.stamp_$(final_build_id): $(native_packages) $(packages)
	rm -rf $(@D)
	mkdir -p $(@D)
	echo copying packages: $^
	echo to: $(@D)
	cd $(@D); $(foreach package,$^, $(build_TAR) xf $($(package)_cached); )
	touch $@

# $PATH is not preserved between ./configure and make by convention. Its
# modification and overriding at ./configure time is (as I understand it)
# supposed to be captured by the AC_{PROG_{,OBJ}CXX,PATH_{PROG,TOOL}} macros,
# which will expand the program names to their full absolute paths. The notable
# exception is command line overriding: ./configure CC=clang, which skips the
# program name expansion step, and works because the user implicitly indicates
# with CC=clang that clang will be available in $PATH at all times, and is most
# likely part of the user's system.
#
# Therefore, when we "seed the autoconf cache"/"override well-known program
# vars" by setting AR=<blah> in our config.site, either one of two things needs
# to be true for the build system to work correctly:
#
#   1. If we refer to the program by name (e.g. AR=riscv64-gnu-linux-ar), the
#      tool needs to be available in $PATH at all times.
#
#   2. If the tool is _**not**_ expected to be available in $PATH at all times
#      (such as is the case for our native_cctools binutils tools), it needs to
#      be referred to by its absolute path, such as would be output by the
#      AC_PATH_{PROG,TOOL} macros.
#
# Minor note: it is also okay to refer to tools by their absolute path even if
# we expect them to be available in $PATH at all times, more specificity does
# not hurt.
$(host_prefix)/share/config.site : config.site.in $(host_prefix)/.stamp_$(final_build_id)
	@mkdir -p $(@D)
	sed -e 's|@HOST@|$(host)|' \
            -e 's|@CC@|$(toolchain_path)$(host_CC)|' \
            -e 's|@CXX@|$(toolchain_path)$(host_CXX)|' \
            -e 's|@AR@|$(toolchain_path)$(host_AR)|' \
            -e 's|@RANLIB@|$(toolchain_path)$(host_RANLIB)|' \
            -e 's|@NM@|$(toolchain_path)$(host_NM)|' \
            -e 's|@STRIP@|$(toolchain_path)$(host_STRIP)|' \
            -e 's|@build_os@|$(build_os)|' \
            -e 's|@host_os@|$(host_os)|' \
            -e 's|@CFLAGS@|$(strip $(host_CFLAGS) $(host_$(release_type)_CFLAGS))|' \
            -e 's|@CXXFLAGS@|$(strip $(host_CXXFLAGS) $(host_$(release_type)_CXXFLAGS))|' \
            -e 's|@CPPFLAGS@|$(strip $(host_CPPFLAGS) $(host_$(release_type)_CPPFLAGS))|' \
            -e 's|@LDFLAGS@|$(strip $(host_LDFLAGS) $(host_$(release_type)_LDFLAGS))|' \
            -e 's|@allow_host_packages@|$(ALLOW_HOST_PACKAGES)|' \
            -e 's|@debug@|$(DEBUG)|' \
            $< > $@
	touch $@

$(host_prefix)/share/toolchain.cmake : toolchain.cmake.in $(host_prefix)/.stamp_$(final_build_id)
	@mkdir -p $(@D)
	sed -e 's|@HOST@|$(host)|' \
            -e 's|@CC@|$(host_CC)|' \
            -e 's|@CXX@|$(host_CXX)|' \
            -e 's|@AR@|$(toolchain_path)$(host_AR)|' \
            -e 's|@RANLIB@|$(toolchain_path)$(host_RANLIB)|' \
            -e 's|@NM@|$(toolchain_path)$(host_NM)|' \
            -e 's|@STRIP@|$(toolchain_path)$(host_STRIP)|' \
            -e 's|@build_os@|$(build_os)|' \
            -e 's|@host_os@|$(host_os)|' \
            -e 's|@CFLAGS@|$(strip $(host_CFLAGS) $(host_$(release_type)_CFLAGS))|' \
            -e 's|@CXXFLAGS@|$(strip $(host_CXXFLAGS) $(host_$(release_type)_CXXFLAGS))|' \
            -e 's|@CPPFLAGS@|$(strip $(host_CPPFLAGS) $(host_$(release_type)_CPPFLAGS))|' \
            -e 's|@LDFLAGS@|$(strip $(host_LDFLAGS) $(host_$(release_type)_LDFLAGS))|' \
            -e 's|@allow_host_packages@|$(ALLOW_HOST_PACKAGES)|' \
            -e 's|@debug@|$(DEBUG)|' \
            -e 's|@release_type@|$(release_type)|' \
            -e 's|@build_tests@|$(build_tests)|' \
            -e 's|@depends@|$(host_cmake)|' \
            -e 's|@prefix@|$($(host_arch)_$(host_os)_prefix)|'\
            -e 's|@arch@|$(host_arch)|'\
            $< > $@
	touch $@

define check_or_remove_cached
  mkdir -p $(BASE_CACHE)/$(host)/$(package) && cd $(BASE_CACHE)/$(host)/$(package); \
  $(build_SHA256SUM) -c $($(package)_cached_checksum) >/dev/null 2>/dev/null || \
  ( rm -f $($(package)_cached_checksum); \
    if test -f "$($(package)_cached)"; then echo "Checksum mismatch for $(package). Forcing rebuild.."; rm -f $($(package)_cached_checksum) $($(package)_cached); fi )
endef

define check_or_remove_sources
  mkdir -p $($(package)_source_dir); cd $($(package)_source_dir); \
  test -f $($(package)_fetched) && ( $(build_SHA256SUM) -c $($(package)_fetched) >/dev/null 2>/dev/null || \
    ( echo "Checksum missing or mismatched for $(package) source. Forcing re-download."; \
      rm -f $($(package)_all_sources) $($(1)_fetched))) || true
endef

check-packages:
	@$(foreach package,$(all_packages),$(call check_or_remove_cached,$(package));)
check-sources:
	@$(foreach package,$(all_packages),$(call check_or_remove_sources,$(package));)

$(host_prefix)/share/config.site: check-packages
$(host_prefix)/share/toolchain.cmake: check-packages

check-packages: check-sources

clean-all: clean
	@rm -rf $(SOURCES_PATH) x86_64* i686* mips* arm* aarch64*

clean:
	@rm -rf $(WORK_PATH) $(BASE_CACHE) $(BUILD) *.log

install: check-packages $(host_prefix)/share/config.site
install: check-packages $(host_prefix)/share/toolchain.cmake

download-one: check-sources $(all_sources)

download-osx:
	@$(MAKE) -s HOST=x86_64-apple-darwin download-one
download-linux:
	@$(MAKE) -s HOST=x86_64-unknown-linux-gnu download-one
download-win:
	@$(MAKE) -s HOST=x86_64-w64-mingw32 download-one
download: download-osx download-linux download-win

$(foreach package,$(all_packages),$(eval $(call ext_add_stages,$(package))))

.PHONY: install cached clean clean-all download-one download-osx download-linux download-win download check-packages check-sources
.PHONY: FORCE
$(V).SILENT:
