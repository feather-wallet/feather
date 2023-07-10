OSX_MIN_VERSION=11.0
OSX_SDK_VERSION=11.0
XCODE_VERSION=12.2
XCODE_BUILD_ID=12B45b
LD64_VERSION=609

OSX_SDK=$(host_prefix)/native/SDK

darwin_native_binutils=native_cctools

darwin_native_toolchain=darwin_sdk

# We can't just use $(shell command -v clang) because GNU Make handles builtins
# in a special way and doesn't know that `command` is a POSIX-standard builtin
# prior to 1af314465e5dfe3e8baa839a32a72e83c04f26ef, first released in v4.2.90.
# At the time of writing, GNU Make v4.2.1 is still being used in supported
# distro releases.
#
# Source: https://lists.gnu.org/archive/html/bug-make/2017-11/msg00017.html
clang_prog=$(shell $(SHELL) $(.SHELLFLAGS) "command -v clang")
clangxx_prog=$(shell $(SHELL) $(.SHELLFLAGS) "command -v clang++")

clang_resource_dir=$(shell clang -print-resource-dir)

cctools_TOOLS=AR RANLIB STRIP NM LIBTOOL OTOOL INSTALL_NAME_TOOL DSYMUTIL

# Make-only lowercase function
lc = $(subst A,a,$(subst B,b,$(subst C,c,$(subst D,d,$(subst E,e,$(subst F,f,$(subst G,g,$(subst H,h,$(subst I,i,$(subst J,j,$(subst K,k,$(subst L,l,$(subst M,m,$(subst N,n,$(subst O,o,$(subst P,p,$(subst Q,q,$(subst R,r,$(subst S,s,$(subst T,t,$(subst U,u,$(subst V,v,$(subst W,w,$(subst X,x,$(subst Y,y,$(subst Z,z,$1))))))))))))))))))))))))))

# For well-known tools provided by cctools, make sure that their well-known
# variable is set to the full path of the tool, just like how AC_PATH_{TOO,PROG}
# would.
$(foreach TOOL,$(cctools_TOOLS),$(eval darwin_$(TOOL) = $$(build_prefix)/bin/$$(host)-$(call lc,$(TOOL))))

# Flag explanations:
#
#     -mlinker-version
#
#         Ensures that modern linker features are enabled. See here for more
#         details: https://github.com/bitcoin/bitcoin/pull/19407.
#
#     -B$(build_prefix)/bin
#
#         Explicitly point to our binaries (e.g. cctools) so that they are
#         ensured to be found and preferred over other possibilities.
#
#     -isysroot$(OSX_SDK) -nostdlibinc
#
#         Disable default include paths built into the compiler as well as
#         those normally included for libc and libc++. The only path that
#         remains implicitly is the clang resource dir.
#
#     -iwithsysroot / -iframeworkwithsysroot
#
#         Adds the desired paths from the SDK
#

darwin_CC_=--target=$(host) -mmacosx-version-min=$(OSX_MIN_VERSION) \
           -B$(build_prefix)/bin -mlinker-version=$(LD64_VERSION) \
           -isysroot$(OSX_SDK) \
           -isysroot$(OSX_SDK) -nostdlibinc \
           -iwithsysroot/usr/include -iframeworkwithsysroot/System/Library/Frameworks
darwin_CC=env -u C_INCLUDE_PATH -u CPLUS_INCLUDE_PATH \
              -u OBJC_INCLUDE_PATH -u OBJCPLUS_INCLUDE_PATH -u CPATH \
              -u LIBRARY_PATH \
            $(clang_prog) $(darwin_CC_)

darwin_CXX_=--target=$(host) -mmacosx-version-min=$(OSX_MIN_VERSION) \
            -B$(build_prefix)/bin -mlinker-version=$(LD64_VERSION) \
            -isysroot$(OSX_SDK) -nostdlibinc \
            -iwithsysroot/usr/include/c++/v1 \
            -iwithsysroot/usr/include -iframeworkwithsysroot/System/Library/Frameworks
darwin_CXX=env -u C_INCLUDE_PATH -u CPLUS_INCLUDE_PATH \
               -u OBJC_INCLUDE_PATH -u OBJCPLUS_INCLUDE_PATH -u CPATH \
               -u LIBRARY_PATH \
             $(clangxx_prog) $(darwin_CXX_)

darwin_CFLAGS=-pipe
darwin_CXXFLAGS=-pipe -std=$(CXX_STANDARD)
darwin_ARFLAGS=cr

darwin_release_CFLAGS=-O2
darwin_release_CXXFLAGS=$(darwin_release_CFLAGS)

darwin_debug_CFLAGS=-O1
darwin_debug_CXXFLAGS=$(darwin_debug_CFLAGS)

darwin_cmake_system=Darwin
