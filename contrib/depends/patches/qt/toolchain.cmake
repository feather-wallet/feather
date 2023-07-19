set(CMAKE_SYSTEM_NAME @cmake_system_name@)

set(CMAKE_C_COMPILER @target@-gcc)
set(CMAKE_CXX_COMPILER @target@-g++)
set(CMAKE_FIND_ROOT_PATH @host_prefix@)

set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(TARGET_SYSROOT @host_prefix@/native/SDK)
    set(CMAKE_SYSROOT ${TARGET_SYSROOT})
    set(CMAKE_OSX_SYSROOT ${TARGET_SYSROOT})

    set(CMAKE_C_COMPILER clang)
    set(CMAKE_CXX_COMPILER clang++)

    set(CMAKE_C_FLAGS "@cmake_c_flags@")
    set(CMAKE_CXX_FLAGS "@cmake_cxx_flags@")

    set(CMAKE_INSTALL_NAME_TOOL @target@-install_name_tool)
endif()