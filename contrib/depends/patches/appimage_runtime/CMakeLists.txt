cmake_minimum_required(VERSION 3.2)
project(appimage_runtime)

# DISCLAIMER:
# Feather builds should be bootstrappable. For this reason we cannot use the runtime binaries provided by the AppImage devs.
# This directory exists to allow the AppImage runtime to be compiled from source using the depends build system.
# We strongly discourage other projects from copying this code, as it WILL NOT produce a standard runtime.
# The runtime produced here is not guaranteed to run on all officially supported platforms.
# You have been warned.

set(CMAKE_VERBOSE_MAKEFILE ON)

# C and C++ versions
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 98)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# sanitizer support
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/sanitizers-cmake/cmake" ${CMAKE_MODULE_PATH})

set(APPIMAGEKIT_RUNTIME_ENABLE_SETPROCTITLE OFF CACHE BOOL "Useful for $TARGET_APPIMAGE; see issue #763")

# check type of current build
string(TOUPPER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_UPPER)
if (BUILD_TYPE_UPPER STREQUAL DEBUG)
    set(BUILD_DEBUG TRUE)
else()
    set(BUILD_DEBUG FALSE)
endif()

# must not include -flto in the following flags, otherwise the data sections will be stripped out
set(runtime_cflags
        -std=c99 -ffunction-sections -fdata-sections
        -I/feather/contrib/depends/x86_64-linux-gnu/include
        -I/feather/contrib/depends/x86_64-linux-gnu/include/appimage
        )
# must not include -Wl,--gc-sections in the following flags, otherwise the data sections will be stripped out
set(runtime_ldflags -s -ffunction-sections -fdata-sections -flto)

if(BUILD_DEBUG)
    message(WARNING "Debug build, adding debug information")
    set(runtime_cflags -g ${runtime_cflags})
else()
    message(STATUS "Release build, optimizing runtime")
    set(runtime_cflags -Os ${runtime_cflags})
endif()

if(APPIMAGEKIT_RUNTIME_ENABLE_SETPROCTITLE)
    set(runtime_cflags ${runtime_cflags} -DENABLE_SETPROCTITLE)
endif()

# objcopy requires actual files for creating new sections to populate the new section
# therefore, we generate 3 suitable files containing blank bytes in the right sizes
add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/16_blank_bytes
        COMMAND dd if=/dev/zero bs=1 count=16 of=${CMAKE_CURRENT_BINARY_DIR}/16_blank_bytes
)
add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/1024_blank_bytes
        COMMAND dd if=/dev/zero bs=1 count=1024 of=${CMAKE_CURRENT_BINARY_DIR}/1024_blank_bytes
)
add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/8192_blank_bytes
        COMMAND dd if=/dev/zero bs=1 count=8192 of=${CMAKE_CURRENT_BINARY_DIR}/8192_blank_bytes
)

# compile first raw object (not linked yet) into which the sections will be embedded
# TODO: find out how this .o object can be generated using a normal add_executable call
# that'd allow us to get rid of the -I parameters in runtime_cflags
add_custom_command(
        MAIN_DEPENDENCY runtime.c
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/runtime.0.o
        COMMAND ${CMAKE_C_COMPILER} ${runtime_cflags} -c ${CMAKE_CURRENT_SOURCE_DIR}/runtime.c -o runtime.0.o
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# embed the sections, one by one
# TODO: find out whether all the sections can be embedded in a single objcopy call
add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/runtime.1.o
        COMMAND objcopy --add-section .digest_md5=16_blank_bytes --set-section-flags .digest_md5=noload,readonly runtime.0.o runtime.1.o
        MAIN_DEPENDENCY ${CMAKE_CURRENT_BINARY_DIR}/runtime.0.o
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/16_blank_bytes
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/runtime.2.o
        COMMAND objcopy --add-section .upd_info=1024_blank_bytes --set-section-flags .upd_info=noload,readonly runtime.1.o runtime.2.o
        MAIN_DEPENDENCY ${CMAKE_CURRENT_BINARY_DIR}/runtime.1.o
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/1024_blank_bytes
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/runtime.3.o
        COMMAND objcopy --add-section .sha256_sig=1024_blank_bytes --set-section-flags .sha256_sig=noload,readonly runtime.2.o runtime.3.o
        MAIN_DEPENDENCY ${CMAKE_CURRENT_BINARY_DIR}/runtime.2.o
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/1024_blank_bytes
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/runtime.4.o
        COMMAND objcopy --add-section .sig_key=8192_blank_bytes --set-section-flags .sig_key=noload,readonly runtime.3.o runtime.4.o
        MAIN_DEPENDENCY ${CMAKE_CURRENT_BINARY_DIR}/runtime.3.o
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/8192_blank_bytes
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# add the runtime as a normal executable
# CLion will recognize it as a normal executable, one can simply step into the code
add_executable(runtime ${CMAKE_CURRENT_BINARY_DIR}/runtime.4.o notify.c)
if(COMMAND target_link_options)
    target_link_options(runtime PRIVATE ${runtime_ldflags})
else()
    message(WARNING "CMake version < 3.13, falling back to using target_link_libraries instead of target_link_options")
    target_link_libraries(runtime PRIVATE ${runtime_ldflags})
endif()
# CMake gets confused by the .o object, therefore we need to tell it that it shall link everything using the C compiler
set_property(TARGET runtime PROPERTY LINKER_LANGUAGE C)
target_link_libraries(runtime PRIVATE -Wl,--as-needed -Wl,--dynamic-linker=/lib64/ld-linux-x86-64.so.2 -static-libstdc++ -Wl,-O2 -zmuldefs -L/feather/contrib/depends/x86_64-linux-gnu/lib squashfuse squashfuse_ll fuseprivate lzma z dl  pthread appimage_shared appimage_hashlib )

target_include_directories(runtime PRIVATE ${PROJECT_SOURCE_DIR}/include)

add_custom_command(
        TARGET runtime
        POST_BUILD
        COMMAND strip ${CMAKE_CURRENT_BINARY_DIR}/runtime
)

add_custom_command(
        TARGET runtime
        POST_BUILD
        COMMAND echo -ne 'AI\\x02' | dd of="${CMAKE_CURRENT_BINARY_DIR}/runtime" bs=1 count=3 seek=8 conv=notrunc
)
