# Targeted operating system.
set(CMAKE_SYSTEM_NAME Darwin)

# TODO: don't hardcode this
set(TARGET_SYSROOT /feather/contrib/depends/arm64-apple-darwin/native/SDK)
set(CMAKE_SYSROOT ${TARGET_SYSROOT})
set(CMAKE_OSX_SYSROOT ${TARGET_SYSROOT})

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

set(CMAKE_C_FLAGS "@cmake_c_flags@")
set(CMAKE_CXX_FLAGS "@cmake_cxx_flags@")

set(CMAKE_INSTALL_NAME_TOOL arm64-apple-darwin-install_name_tool)
set(CMAKE_FIND_ROOT_PATH /feather/contrib/depends/arm64-apple-darwin/)

# Adjust the default behavior of the find commands:
# search headers and libraries in the target environment
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)

# Search programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
