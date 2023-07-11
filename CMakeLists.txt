cmake_minimum_required(VERSION 3.18)

project(feather
  VERSION "2.4.9"
  DESCRIPTION "A free Monero desktop wallet"
  LANGUAGES CXX C ASM
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PACKAGE_NAME ${PROJECT_NAME})
set(PACKAGE_BUGREPORT "https://github.com/feather-wallet/feather/issues")
set(PACKAGE_URL "https://featherwallet.org/")
set(COPYRIGHT_YEAR "2023")
set(COPYRIGHT_HOLDERS "The Monero Project")

# Configurable options
option(STATIC "Link libraries statically, requires static Qt" OFF)
option(SELF_CONTAINED "Disable when building Feather for packages" OFF)
option(LOCALMONERO "Include LocalMonero module" ON)
option(XMRIG "Include XMRig module" ON)
option(TOR_DIR "Directory containing Tor binaries to embed inside Feather" OFF)
option(CHECK_UPDATES "Enable checking for application updates" OFF)
option(PLATFORM_INSTALLER "Built-in updater fetches installer (windows-only)" OFF)
option(USE_DEVICE_TREZOR "Trezor support compilation" ON)
option(DONATE_BEG "Prompt donation window every once in a while" OFF)
option(WITH_SCANNER "Enable webcam QR scanner" ON)
option(STACK_TRACE "Dump stack trace on crash (Linux only)" OFF)

list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_SOURCE_DIR}/cmake")
include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)
include(CheckIncludeFile)
include(CheckSymbolExists)
include(SelectLibraryConfigurations)

if(DEBUG)
    set(CMAKE_VERBOSE_MAKEFILE ON)
endif()

# Monero configuration
set(BUILD_GUI_DEPS ON)
set(BUILD_64 ON)
set(USE_SINGLE_BUILDDIR ON)

if(STATIC)
    message(STATUS "Initiating static build, turning on manual submodules")
    set(MANUAL_SUBMODULES 1)

    set(Boost_USE_STATIC_LIBS ON)
    set(Boost_USE_STATIC_RUNTIME ON)
endif()

include(CMakePackageConfigHelpers)
include(VersionFeather)

#### Dependencies ####
# Monero
if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/monero/CMakeLists.txt")
    message(FATAL_ERROR "'monero/CMakeLists.txt' does not exist, did you forget to:\ngit submodule update --init --recursive --progress")
endif()
add_subdirectory(monero EXCLUDE_FROM_ALL)
set_property(TARGET wallet_merged PROPERTY FOLDER "monero")
get_directory_property(ARCH_WIDTH DIRECTORY "monero" DEFINITION ARCH_WIDTH)
get_directory_property(UNBOUND_LIBRARY DIRECTORY "monero" DEFINITION UNBOUND_LIBRARY)
get_directory_property(DEVICE_TREZOR_READY DIRECTORY "monero" DEFINITION DEVICE_TREZOR_READY)
get_directory_property(TREZOR_DEP_LIBS DIRECTORY "monero" DEFINITION TREZOR_DEP_LIBS)

# libXau
# workaround for: undefined reference to `XauGetBestAuthByAddr'
if (DEPENDS AND UNIX AND NOT APPLE)
    find_library(LIBXAU_LIBRARY Xau)
    message(STATUS "libXau: libraries at ${LIBXAU_LIBRARY}")
endif()

# pthread
find_package(Threads REQUIRED)

# Sodium
find_library(SODIUM_LIBRARY sodium)
message(STATUS "libsodium: libraries at ${SODIUM_LIBRARY}")

# QrEncode
find_package(QREncode REQUIRED)

# Polyseed
find_package(Polyseed REQUIRED)
if(Polyseed_SUBMODULE)
    add_subdirectory(src/third-party/polyseed EXCLUDE_FROM_ALL)
endif()

# ZBAR
if(WITH_SCANNER)
    find_package(ZBAR REQUIRED)
    message(STATUS "libzbar: include dir at ${ZBAR_INCLUDE_DIR}")
    message(STATUS "libzbar: libraries at ${ZBAR_LIBRARIES}")
endif()

# libzip
if(CHECK_UPDATES)
    set(ZLIB_USE_STATIC_LIBS "ON")
    find_package(ZLIB REQUIRED)
    find_path(LIBZIP_INCLUDE_DIRS zip.h)
    find_library(LIBZIP_LIBRARIES zip)
endif()

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

set(BOOST_COMPONENTS
        system
        filesystem
        thread
        date_time
        chrono
        regex
        serialization
        program_options
        locale
)

if(STACK_TRACE AND UNIX AND NOT APPLE)
    list(APPEND BOOST_COMPONENTS
            stacktrace_basic)
endif()

set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.58 REQUIRED COMPONENTS ${BOOST_COMPONENTS})

if(UNIX AND NOT APPLE)
    if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        # https://github.com/monero-project/monero-gui/issues/3142#issuecomment-705940446
        set(CMAKE_SKIP_RPATH ON)
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
    message(STATUS "Skipping Tor inclusion because -DTOR_DIR=Off")
endif()

message(STATUS "Using Boost include dir at ${Boost_INCLUDE_DIRS}")
message(STATUS "Using Boost libraries at ${Boost_LIBRARIES}")

include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
if(MINGW)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wa,-mbig-obj")
    set(EXTRA_LIBRARIES mswsock;ws2_32;iphlpapi;crypt32;bcrypt)
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

if(APPLE)
    cmake_policy(SET CMP0042 NEW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=default -std=c++17")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=default -DGTEST_HAS_TR1_TUPLE=0")
endif()

#### Security Flags ####
# warnings
add_c_flag_if_supported(-Wformat C_SECURITY_FLAGS)
add_cxx_flag_if_supported(-Wformat CXX_SECURITY_FLAGS)
add_c_flag_if_supported(-Wformat-security C_SECURITY_FLAGS)
add_cxx_flag_if_supported(-Wformat-security CXX_SECURITY_FLAGS)

# -fstack-protector
if (NOT OPENBSD)
    add_c_flag_if_supported(-fstack-protector C_SECURITY_FLAGS)
    add_cxx_flag_if_supported(-fstack-protector CXX_SECURITY_FLAGS)
    add_c_flag_if_supported(-fstack-protector-strong C_SECURITY_FLAGS)
    add_cxx_flag_if_supported(-fstack-protector-strong CXX_SECURITY_FLAGS)
endif()

# -fcf-protection=full
# new in GCC 8.2
if (NOT OPENBSD AND NOT (WIN32 AND (CMAKE_C_COMPILER_ID STREQUAL "GNU" AND CMAKE_C_COMPILER_VERSION VERSION_LESS 9.1)))
    add_c_flag_if_supported(-fcf-protection=full C_SECURITY_FLAGS)
    add_cxx_flag_if_supported(-fcf-protection=full CXX_SECURITY_FLAGS)
endif()

# -fstack-clash-protection
if (NOT WIN32 AND NOT OPENBSD AND NOT "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    add_c_flag_if_supported(-fstack-clash-protection C_SECURITY_FLAGS)
    add_cxx_flag_if_supported(-fstack-clash-protection CXX_SECURITY_FLAGS)
endif()

# -mmitigate-rop
# removed in GCC 9.1 (or before ?), but still accepted, so spams the output
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
#    add_linker_flag_if_supported(-static-libgcc STATIC_FLAGS)
#    add_linker_flag_if_supported(-static-libstdc++ STATIC_FLAGS)
    if(MINGW)
        add_linker_flag_if_supported(-static STATIC_FLAGS)
    endif()
endif()

add_c_flag_if_supported(-fPIC C_SECURITY_FLAGS)
add_cxx_flag_if_supported(-fPIC CXX_SECURITY_FLAGS)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 ${C_SECURITY_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_SECURITY_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LD_SECURITY_FLAGS} ${STATIC_FLAGS}")

add_subdirectory(src)

configure_file("${CMAKE_SOURCE_DIR}/contrib/installers/windows/setup.nsi.in" "${CMAKE_SOURCE_DIR}/contrib/installers/windows/setup.nsi" @ONLY)

if(APPLE)
    configure_file(${CMAKE_SOURCE_DIR}/contrib/macdeploy/Info.plist.in ${CMAKE_SOURCE_DIR}/contrib/macdeploy/Info.plist @ONLY)

    set_target_properties(feather PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/contrib/macdeploy/Info.plist"
            LINK_FLAGS_RELEASE -s
    )

    file(COPY "${CMAKE_SOURCE_DIR}/src/assets/images/appicons/appicon.icns" DESTINATION "${CMAKE_SOURCE_DIR}/installed/feather.app/Contents/Resources/" )
endif()

#### Summary ####

message("\n")
message("Configure summary")
message("=================")

if(CMAKE_CROSSCOMPILING)
    set(cross_status "TRUE, for ${CMAKE_SYSTEM_NAME}, ${CMAKE_SYSTEM_PROCESSOR}")
else()
    set(cross_status "FALSE")
endif()

message("Cross compiling ....................... ${cross_status}")
get_target_property(definitions feather COMPILE_DEFINITIONS)
message("Preprocessor defined macros ........... ${definitions}")
message("C compiler ............................ ${CMAKE_C_COMPILER}")
message("CFLAGS ................................ ${CMAKE_C_FLAGS}")
message("C++ compiler .......................... ${CMAKE_CXX_COMPILER}")
message("CXXFLAGS .............................. ${CMAKE_CXX_FLAGS}")
message("LDFLAGS for executables ............... ${CMAKE_EXE_LINKER_FLAGS}")
message("\n")
