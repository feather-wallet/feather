cmake_minimum_required(VERSION 3.20)
project(feather)

message(STATUS "Initiating compile using CMake ${CMAKE_VERSION}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(VERSION_MAJOR "2")
set(VERSION_MINOR "0")
set(VERSION_REVISION "0")
set(VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_REVISION}")

option(STATIC "Link libraries statically, requires static Qt")

option(SELF_CONTAINED "Disable when building Feather for packages" OFF)
option(LOCALMONERO "Include LocalMonero module" ON)
option(XMRIG "Include XMRig module" ON)
option(TOR_BIN "Path to Tor binary to embed inside Feather" OFF)
option(CHECK_UPDATES "Enable checking for application updates" OFF)
option(PLATFORM_INSTALLER "Built-in updater fetches installer (windows-only)" OFF)
option(USE_DEVICE_TREZOR "Trezor support compilation" ON)
option(DONATE_BEG "Prompt donation window every once in a while" ON)
option(WITH_SCANNER "Enable webcam QR scanner" OFF)

list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_SOURCE_DIR}/cmake")
include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)
include(CheckIncludeFile)
include(CheckSymbolExists)

if(DEBUG)
    set(CMAKE_VERBOSE_MAKEFILE ON)
endif()

set(BUILD_GUI_DEPS ON)
option(ARCH "Target architecture" "native")
set(BUILD_64 ON)
set(USE_SINGLE_BUILDDIR ON)

check_include_file(sys/prctl.h HAVE_SYS_PRCTL_H)
check_symbol_exists(prctl "sys/prctl.h" HAVE_PRCTL)

if(STATIC)
    message(STATUS "Initiating static build, turning on manual submodules")
    set(MANUAL_SUBMODULES 1)

    set(Boost_USE_STATIC_LIBS ON)
    set(Boost_USE_STATIC_RUNTIME ON)
endif()

add_subdirectory(monero)
set_property(TARGET wallet_merged PROPERTY FOLDER "monero")
get_directory_property(ARCH_WIDTH DIRECTORY "monero" DEFINITION ARCH_WIDTH)
get_directory_property(UNBOUND_LIBRARY DIRECTORY "monero" DEFINITION UNBOUND_LIBRARY)
get_directory_property(DEVICE_TREZOR_READY DIRECTORY "monero" DEFINITION DEVICE_TREZOR_READY)
get_directory_property(TREZOR_DEP_LIBS DIRECTORY "monero" DEFINITION TREZOR_DEP_LIBS)

include(CMakePackageConfigHelpers)
include(VersionFeather)

include_directories(${EASYLOGGING_INCLUDE})
link_directories(${EASYLOGGING_LIBRARY_DIRS})

# Sodium
find_library(SODIUM_LIBRARY sodium)
message(STATUS "libsodium: libraries at ${SODIUM_LIBRARY}")

# QrEncode
find_package(QREncode REQUIRED)

# Qr scanner
find_package(ZBAR REQUIRED)
message(STATUS "libzbar: include dir at ${ZBAR_INCLUDE_DIR}")
message(STATUS "libzbar: libraries at ${ZBAR_LIBRARIES}")

# Tevador 14 word Monero seed
add_subdirectory(contrib/monero-seed)

# Polyseed 16 word mnemonic seeds
find_package(Polyseed REQUIRED)

# libzip
find_package(zlib CONFIG)
find_path(LIBZIP_INCLUDE_DIRS zip.h)
find_library(LIBZIP_LIBRARIES zip)

# Boost
if(DEBUG)
    set(Boost_DEBUG ON)
endif()
if(APPLE AND NOT BOOST_ROOT)
    execute_process(COMMAND brew --prefix boost OUTPUT_VARIABLE BOOST_ROOT OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()
if(MINGW)
    set(Boost_THREADAPI win32)
endif()
find_package(Boost 1.58 REQUIRED COMPONENTS
        system
        filesystem
        thread
        date_time
        chrono
        regex
        serialization
        program_options
        locale)

if(UNIX AND NOT APPLE)
    if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        # https://github.com/monero-project/monero-gui/issues/3142#issuecomment-705940446
        set(CMAKE_SKIP_RPATH ON)
    endif()

    find_package(X11 REQUIRED)
    message(STATUS "X11_FOUND = ${X11_FOUND}")
    message(STATUS "X11_INCLUDE_DIR = ${X11_INCLUDE_DIR}")
    message(STATUS "X11_LIBRARIES = ${X11_LIBRARIES}")
    include_directories(${X11_INCLUDE_DIR})
    link_directories(${X11_LIBRARIES})
    if(STATIC)
        find_library(XCB_LIBRARY xcb)
        message(STATUS "Found xcb library: ${XCB_LIBRARY}")
    endif()
endif()

include(TorQrcGenerator)

# To build Feather with embedded (and static) Tor, pass CMake -DTOR_DIR=/path/to/tor/
if(TOR_DIR)
    if (NOT TOR_VERSION)
        message(FATAL_ERROR "TOR_DIR is specified but TOR_VERSION is not set")
    endif()

    message(STATUS "Embedded Tor version: ${TOR_VERSION}")
    configure_file("cmake/config-feather.h.cmake" "${CMAKE_CURRENT_SOURCE_DIR}/src/config-feather.h")

    # Always copy Tor when doing a reproducible build to prevent old versions from getting included
    if (REPRODUCIBLE)
        set(TOR_COPY_CMD "cp -a ${TOR_DIR}/* ${CMAKE_CURRENT_SOURCE_DIR}/src/assets/tor")
    else()
        set(TOR_COPY_CMD "cp -au ${TOR_DIR}/* ${CMAKE_CURRENT_SOURCE_DIR}/src/assets/tor")
    endif()

    execute_process(COMMAND bash -c "${TOR_COPY_CMD}" RESULT_VARIABLE ret)
    if(ret EQUAL "1")
        message(FATAL_ERROR "Tor copy failure: ${TOR_COPY_CMD}")
    endif()

    message(STATUS "Embedding Tor binaries at ${TOR_DIR}")
else()
    message(STATUS "Skipping Tor inclusion because -DTOR_BIN=Off")
endif()

if(MINGW)
    string(REGEX MATCH "^[^/]:/[^/]*" msys2_install_path "${CMAKE_C_COMPILER}")
    message(STATUS "MSYS location: ${msys2_install_path}")
    set(CMAKE_INCLUDE_PATH "${msys2_install_path}/mingw${ARCH_WIDTH}/include")
    # This is necessary because otherwise CMake will make Boost libraries -lfoo
    # rather than a full path. Unfortunately, this makes the shared libraries get
    # linked due to a bug in CMake which misses putting -static flags around the
    # -lfoo arguments.
    set(DEFLIB ${msys2_install_path}/mingw${ARCH_WIDTH}/lib)
    list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_DIRECTORIES ${DEFLIB})
    list(REMOVE_ITEM CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES ${DEFLIB})
endif()

message(STATUS "Using Boost include dir at ${Boost_INCLUDE_DIRS}")
message(STATUS "Using Boost libraries at ${Boost_LIBRARIES}")

include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
if(MINGW)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wa,-mbig-obj")
    set(EXTRA_LIBRARIES mswsock;ws2_32;iphlpapi;crypt32;bcrypt)
    if(DEPENDS)
        set(ICU_LIBRARIES iconv)
    else()
        set(ICU_LIBRARIES icuio icuin icuuc icudt icutu iconv)
    endif()
elseif(APPLE)
    set(EXTRA_LIBRARIES "-framework AppKit")
elseif(OPENBSD)
    set(EXTRA_LIBRARIES "")
elseif(FREEBSD)
    set(EXTRA_LIBRARIES execinfo)
elseif(DRAGONFLY)
    find_library(COMPAT compat)
    set(EXTRA_LIBRARIES execinfo ${COMPAT})
elseif(CMAKE_SYSTEM_NAME MATCHES "(SunOS|Solaris)")
    set(EXTRA_LIBRARIES socket nsl resolv)
elseif(NOT MSVC AND NOT DEPENDS)
    find_library(RT rt)
    set(EXTRA_LIBRARIES ${RT})
endif()

list(APPEND EXTRA_LIBRARIES ${CMAKE_DL_LIBS})

if(APPLE)
    cmake_policy(SET CMP0042 NEW)
endif()

if (APPLE AND NOT IOS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=default -std=c++11")
endif()

if(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=default -DGTEST_HAS_TR1_TUPLE=0")
endif()

# warnings
add_c_flag_if_supported(-Wformat C_SECURITY_FLAGS)
add_cxx_flag_if_supported(-Wformat CXX_SECURITY_FLAGS)
add_c_flag_if_supported(-Wformat-security C_SECURITY_FLAGS)
add_cxx_flag_if_supported(-Wformat-security CXX_SECURITY_FLAGS)

# -fstack-protector
if (NOT OPENBSD AND NOT (WIN32 AND (CMAKE_C_COMPILER_ID STREQUAL "GNU" AND CMAKE_C_COMPILER_VERSION VERSION_LESS 9.1)))
    add_c_flag_if_supported(-fstack-protector C_SECURITY_FLAGS)
    add_cxx_flag_if_supported(-fstack-protector CXX_SECURITY_FLAGS)
    add_c_flag_if_supported(-fstack-protector-strong C_SECURITY_FLAGS)
    add_cxx_flag_if_supported(-fstack-protector-strong CXX_SECURITY_FLAGS)
endif()

# New in GCC 8.2
if (NOT OPENBSD AND NOT (WIN32 AND (CMAKE_C_COMPILER_ID STREQUAL "GNU" AND CMAKE_C_COMPILER_VERSION VERSION_LESS 9.1)))
    add_c_flag_if_supported(-fcf-protection=full C_SECURITY_FLAGS)
    add_cxx_flag_if_supported(-fcf-protection=full CXX_SECURITY_FLAGS)
endif()
if (NOT WIN32 AND NOT OPENBSD)
    add_c_flag_if_supported(-fstack-clash-protection C_SECURITY_FLAGS)
    add_cxx_flag_if_supported(-fstack-clash-protection CXX_SECURITY_FLAGS)
endif()

# Removed in GCC 9.1 (or before ?), but still accepted, so spams the output
if (NOT (CMAKE_C_COMPILER_ID STREQUAL "GNU" AND NOT CMAKE_C_COMPILER_VERSION VERSION_LESS 9.1))
    add_c_flag_if_supported(-mmitigate-rop C_SECURITY_FLAGS)
    add_cxx_flag_if_supported(-mmitigate-rop CXX_SECURITY_FLAGS)
endif()

# linker
if (APPLE)
    add_linker_flag_if_supported(-Wl,-bind_at_load LD_SECURITY_FLAGS)
    add_linker_flag_if_supported(-Wl,-dead_strip LD_SECURITY_FLAGS)
    add_linker_flag_if_supported(-Wl,-dead_strip_dylibs LD_SECURITY_FLAGS)
endif()
if (NOT APPLE AND NOT (WIN32 AND CMAKE_C_COMPILER_ID STREQUAL "GNU"))
    # Windows binaries die on startup with PIE when compiled with GCC
    add_linker_flag_if_supported(-pie LD_SECURITY_FLAGS)
endif()
add_linker_flag_if_supported(-Wl,-z,relro LD_SECURITY_FLAGS)
add_linker_flag_if_supported(-Wl,-z,now LD_SECURITY_FLAGS)
add_linker_flag_if_supported(-Wl,-z,noexecstack noexecstack_SUPPORTED)
if (noexecstack_SUPPORTED)
    set(LD_SECURITY_FLAGS "${LD_SECURITY_FLAGS} -Wl,-z,noexecstack")
endif()
add_linker_flag_if_supported(-Wl,-z,noexecheap noexecheap_SUPPORTED)
if (noexecheap_SUPPORTED)
    set(LD_SECURITY_FLAGS "${LD_SECURITY_FLAGS} -Wl,-z,noexecheap")
endif()

# some windows linker bits
if (WIN32)
    add_linker_flag_if_supported(-Wl,--dynamicbase LD_SECURITY_FLAGS)
    add_linker_flag_if_supported(-Wl,--nxcompat LD_SECURITY_FLAGS)
    add_linker_flag_if_supported(-Wl,--high-entropy-va LD_SECURITY_FLAGS)
endif()

if(STATIC)
    add_linker_flag_if_supported(-static-libgcc STATIC_FLAGS)
    add_linker_flag_if_supported(-static-libstdc++ STATIC_FLAGS)
    if(MINGW)
        add_linker_flag_if_supported(-static STATIC_FLAGS)
    endif()
endif()

# With GCC 6.1.1 the compiled binary malfunctions due to aliasing. Until that
# is fixed in the code (Issue #847), force compiler to be conservative.
add_c_flag_if_supported(-fno-strict-aliasing C_SECURITY_FLAGS)
add_cxx_flag_if_supported(-fno-strict-aliasing CXX_SECURITY_FLAGS)

add_c_flag_if_supported(-fPIC C_SECURITY_FLAGS)
add_cxx_flag_if_supported(-fPIC CXX_SECURITY_FLAGS)

message(STATUS "Using C security hardening flags: ${C_SECURITY_FLAGS}")
message(STATUS "Using C++ security hardening flags: ${CXX_SECURITY_FLAGS}")
message(STATUS "Using linker security hardening flags: ${LD_SECURITY_FLAGS}")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 ${C_SECURITY_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 ${CXX_SECURITY_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LD_SECURITY_FLAGS} ${STATIC_FLAGS}")

add_subdirectory(src)
