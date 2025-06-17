cmake_minimum_required(VERSION 3.16)

get_filename_component(LIBUSB_ROOT "libusb" ABSOLUTE)

# Get the version information from version.h ignoring the nano version as it appears in version_nano.h and so we need it?
file(READ "${LIBUSB_ROOT}/version.h" VERSIONHEADERDATA)
string(REGEX MATCH "#define LIBUSB_MAJOR ([0-9]*)" _ ${VERSIONHEADERDATA})
set(LIBUSB_VERSION_MAJOR ${CMAKE_MATCH_1})
string(REGEX MATCH "#define LIBUSB_MINOR ([0-9]*)" _ ${VERSIONHEADERDATA})
set(LIBUSB_VERSION_MINOR ${CMAKE_MATCH_1})
string(REGEX MATCH "#define LIBUSB_MICRO ([0-9]*)" _ ${VERSIONHEADERDATA})
set(LIBUSB_VERSION_MICRO ${CMAKE_MATCH_1})
set(LIBUSB_VERSION "${LIBUSB_VERSION_MAJOR}.${LIBUSB_VERSION_MINOR}.${LIBUSB_VERSION_MICRO}")

project(usb-1.0
        DESCRIPTION "A cross-platform library to access USB devices"
        VERSION ${LIBUSB_VERSION}
        LANGUAGES C
)
if(EMSCRIPTEN)
    set(CMAKE_CXX_STANDARD 20)
    enable_language(CXX)
endif()

# This function generates all the local variables what end up getting written to config.
# We use a function as any vars set in this context don't mess with the rest of the file.
# e.g. Logging LIBUSB_ENABLE_LOGGING mapps to ENABLE_LOGGING in the config, keeps it clean
function(generate_config_file)
    include(CheckIncludeFiles)
    include(CheckFunctionExists)
    include(CheckSymbolExists)
    include(CheckStructHasMember)
    include(CheckCCompilerFlag)

    check_function_exists(clock_gettime             HAVE_CLOCK_GETTIME)
    check_function_exists(pthread_condattr_setclock HAVE_PTHREAD_CONDATTR_SETCLOCK)
    check_function_exists(pthread_setname_np        HAVE_PTHREAD_SETNAME_NP)
    check_function_exists(pthread_threadid_np       HAVE_PTHREAD_THREADID_NP)
    check_function_exists(eventfd                   HAVE_EVENTFD)
    check_function_exists(pipe2                     HAVE_PIPE2)
    check_function_exists(syslog                    HAVE_SYSLOG)

    check_include_files(asm/types.h      HAVE_ASM_TYPES_H)
    check_include_files(sys/eventfd.h    HAVE_EVENTFD)
    check_include_files(string.h         HAVE_STRING_H)
    check_include_files(sys/time.h       HAVE_SYS_TIME_H)

    check_symbol_exists(timerfd_create  "sys/timerfd.h" HAVE_TIMERFD)
    check_symbol_exists(nfds_t  "poll.h" HAVE_NFDS_T)

    check_struct_has_member("struct timespec" tv_sec time.h HAVE_STRUCT_TIMESPEC)

    if(HAVE_VISIBILITY)
        set(DEFAULT_VISIBILITY "__attribute__((visibility(\"default\")))")
    else()
        set(DEFAULT_VISIBILITY "" )
    endif()

    # Set vars that will be written into the config file.
    if(WIN32)
        set(PLATFORM_WINDOWS 1)
    else()
        set(PLATFORM_POSIX 1)
    endif()

    if(LIBUSB_ENABLE_LOGGING)
        set(ENABLE_LOGGING ${LIBUSB_ENABLE_LOGGING})
    endif()
    if(LIBUSB_ENABLE_DEBUG_LOGGING)
        set(ENABLE_DEBUG_LOGGING ${LIBUSB_ENABLE_DEBUG_LOGGING})
    endif()

    if(CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_C_COMPILER_ID STREQUAL "GNU")
        check_c_compiler_flag("-fvisibility=hidden" HAVE_VISIBILITY)
    endif()

    file(MAKE_DIRECTORY "${LIBUSB_GEN_INCLUDES}")
    if(NOT MSVC)
        set(_GNU_SOURCE TRUE)
    endif()
    configure_file(config.h.in "${LIBUSB_GEN_INCLUDES}/config.h" @ONLY)
endfunction()

if(BUILD_SHARED_LIBS)
    set(LIBUSB_BUILD_SHARED_LIBS_DEFAULT ON)
else()
    set(LIBUSB_BUILD_SHARED_LIBS_DEFAULT OFF)
endif()

option(LIBUSB_BUILD_SHARED_LIBS "Build Shared Libraries for libusb" ${LIBUSB_BUILD_SHARED_LIBS_DEFAULT})
option(LIBUSB_BUILD_TESTING "Build Tests" OFF)
if(LIBUSB_BUILD_TESTING)
    enable_testing()
endif()

option(LIBUSB_BUILD_EXAMPLES "Build Example Applications" OFF)

option(LIBUSB_INSTALL_TARGETS "Install libusb targets" ON)
option(LIBUSB_TARGETS_INCLUDE_USING_SYSTEM "Make targets include paths System" ON)

option(LIBUSB_ENABLE_LOGGING "Enable Logging" ON)
option(LIBUSB_ENABLE_DEBUG_LOGGING "Enable Debug Logging" OFF)
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    option(LIBUSB_ENABLE_UDEV "Enable udev backend for device enumeration" ON)
endif()

set(LIBUSB_GEN_INCLUDES "${CMAKE_CURRENT_BINARY_DIR}/gen_include")
generate_config_file()

if(LIBUSB_BUILD_SHARED_LIBS)
    add_library(usb-1.0 SHARED)
else()
    add_library(usb-1.0 STATIC)
endif()

set_target_properties(usb-1.0 PROPERTIES
        PREFIX lib # to be consistent with mainline libusb build system(s)
)

# common sources
target_sources(usb-1.0 PRIVATE
        "${LIBUSB_GEN_INCLUDES}/config.h"
        "${LIBUSB_ROOT}/core.c"
        "${LIBUSB_ROOT}/descriptor.c"
        "${LIBUSB_ROOT}/hotplug.c"
        "${LIBUSB_ROOT}/io.c"
        "${LIBUSB_ROOT}/libusb.h"
        "${LIBUSB_ROOT}/libusbi.h"
        "${LIBUSB_ROOT}/strerror.c"
        "${LIBUSB_ROOT}/sync.c"
        "${LIBUSB_ROOT}/version.h"
        "${LIBUSB_ROOT}/version_nano.h"
)
target_include_directories(usb-1.0
        PRIVATE
        "${LIBUSB_GEN_INCLUDES}"
        "${LIBUSB_ROOT}/os"
)

if (LIBUSB_TARGETS_INCLUDE_USING_SYSTEM)
    target_include_directories(usb-1.0 SYSTEM PUBLIC "${LIBUSB_ROOT}")
else()
    target_include_directories(usb-1.0 PUBLIC "${LIBUSB_ROOT}")
endif()

if(WIN32)
    target_sources(usb-1.0 PRIVATE
            "${LIBUSB_ROOT}/libusb-1.0.def"
            "${LIBUSB_ROOT}/os/events_windows.c"
            "${LIBUSB_ROOT}/os/events_windows.h"
            "${LIBUSB_ROOT}/os/threads_windows.c"
            "${LIBUSB_ROOT}/os/threads_windows.h"
            "${LIBUSB_ROOT}/os/windows_common.c"
            "${LIBUSB_ROOT}/os/windows_common.h"
            "${LIBUSB_ROOT}/os/windows_usbdk.c"
            "${LIBUSB_ROOT}/os/windows_usbdk.h"
            "${LIBUSB_ROOT}/os/windows_winusb.c"
            "${LIBUSB_ROOT}/os/windows_winusb.h"
            $<$<C_COMPILER_ID:MSVC>:${LIBUSB_ROOT}/libusb-1.0.rc>
    )
    target_compile_definitions(usb-1.0 PRIVATE $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS=1>)
    target_link_libraries(usb-1.0 PRIVATE windowsapp)
    set_target_properties(usb-1.0 PROPERTIES UNITY_BUILD OFF)
else()
    # common POSIX/non-Windows sources
    target_sources(usb-1.0 PRIVATE
            "${LIBUSB_ROOT}/os/events_posix.c"
            "${LIBUSB_ROOT}/os/events_posix.h"
            "${LIBUSB_ROOT}/os/threads_posix.c"
            "${LIBUSB_ROOT}/os/threads_posix.h"
    )
    if(CMAKE_SYSTEM_NAME MATCHES "Linux")
        target_sources(usb-1.0 PRIVATE
                "${LIBUSB_ROOT}/os/linux_usbfs.c"
                "${LIBUSB_ROOT}/os/linux_usbfs.h"
        )
        if(LIBUSB_ENABLE_UDEV)
            target_sources(usb-1.0 PRIVATE
                    "${LIBUSB_ROOT}/os/linux_udev.c"
            )
            target_link_libraries(usb-1.0 PRIVATE udev)
            target_compile_definitions(usb-1.0 PRIVATE HAVE_LIBUDEV=1)
        else()
            target_sources(usb-1.0 PRIVATE
                    "${LIBUSB_ROOT}/os/linux_netlink.c"
            )
        endif()
        find_package(Threads REQUIRED)
        target_link_libraries(usb-1.0 PRIVATE Threads::Threads)
    elseif(ANDROID)
        target_sources(usb-1.0 PRIVATE
                "${LIBUSB_ROOT}/os/linux_netlink.c"
                "${LIBUSB_ROOT}/os/linux_usbfs.c"
                "${LIBUSB_ROOT}/os/linux_usbfs.h"
        )
        target_link_libraries(usb-1.0 PRIVATE android log)
    elseif(APPLE)
        target_sources(usb-1.0 PRIVATE
                "${LIBUSB_ROOT}/os/darwin_usb.c"
                "${LIBUSB_ROOT}/os/darwin_usb.h"
        )
        target_link_libraries(usb-1.0 PRIVATE
                "-framework Foundation"
                "-framework IOKit"
                "-framework Security"
        )
    elseif(CMAKE_SYSTEM_NAME STREQUAL "NetBSD")
        target_sources(usb-1.0 PRIVATE
                "${LIBUSB_ROOT}/os/netbsd_usb.c"
        )
    elseif(CMAKE_SYSTEM_NAME STREQUAL "OpenBSD")
        target_sources(usb-1.0 PRIVATE
                "${LIBUSB_ROOT}/os/openbsd_usb.c"
        )
    elseif(EMSCRIPTEN)
        target_sources(usb-1.0 PRIVATE
                "${LIBUSB_ROOT}/os/emscripten_webusb.cpp"
        )
        target_compile_options(usb-1.0 PRIVATE -pthread)
    else()
        message(FATAL_ERROR "Unsupported target platform: ${CMAKE_SYSTEM_NAME}")
    endif()
endif()

if(LIBUSB_BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(LIBUSB_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(LIBUSB_INSTALL_TARGETS)
    install(TARGETS usb-1.0)
    install(FILES "${LIBUSB_ROOT}/libusb.h" DESTINATION "include/libusb-1.0")
endif()
