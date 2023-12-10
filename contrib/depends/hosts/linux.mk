linux_CFLAGS=-pipe
linux_CXXFLAGS=-pipe -std=$(CXX_STANDARD)
linux_ARFLAGS=cr

linux_release_CFLAGS=-O2
linux_release_CXXFLAGS=$(linux_release_CFLAGS)

linux_debug_CFLAGS=-O1
linux_debug_CXXFLAGS=$(linux_debug_CFLAGS)

linux_debug_CPPFLAGS=-D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -D_LIBCPP_DEBUG=1

ifeq (86,$(findstring 86,$(build_arch)))
x86_64_linux_CC=gcc -m64
x86_64_linux_CXX=g++ -m64
x86_64_linux_AR=ar
x86_64_linux_RANLIB=ranlib
x86_64_linux_NM=nm
x86_64_linux_STRIP=strip
else
x86_64_linux_CC=$(default_host_CC) -m64
x86_64_linux_CXX=$(default_host_CXX) -m64
endif
linux_cmake_system=Linux
