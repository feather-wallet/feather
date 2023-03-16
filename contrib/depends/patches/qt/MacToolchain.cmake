# Targeted operating system.
set(CMAKE_SYSTEM_NAME Darwin)

# TODO: don't hardcode this
set(TARGET_SYSROOT /feather/contrib/depends/x86_64-apple-darwin/native/SDK)
set(CMAKE_SYSROOT ${TARGET_SYSROOT})
set(CMAKE_OSX_SYSROOT ${TARGET_SYSROOT})

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

# TODO: don't hardcode this
set(CMAKE_C_FLAGS "--target=x86_64-apple-darwin -mmacosx-version-min=10.14 -B/feather/contrib/depends/x86_64-apple-darwin/native/bin -mlinker-version=609 -isysroot/feather/contrib/depends/x86_64-apple-darwin/native/SDK -Xclang -internal-externc-isystem/gnu/store/rwsysyzpxzwi7g1jv4hxwi3m8i36iwcc-clang-10.0.1/lib/clang/10.0.1/include -Xclang -internal-externc-isystem/feather/contrib/depends/x86_64-apple-darwin/native/SDK/usr/include")
set(CMAKE_CXX_FLAGS "--target=x86_64-apple-darwin -mmacosx-version-min=10.14 -B/feather/contrib/depends/x86_64-apple-darwin/native/bin -mlinker-version=609 -isysroot/feather/contrib/depends/x86_64-apple-darwin/native/SDK -stdlib=libc++ -stdlib++-isystem/feather/contrib/depends/x86_64-apple-darwin/native/SDK/usr/include/c++/v1 -isystem/feather/contrib/depends/x86_64-apple-darwin/native/SDK/usr/include/c++/v1 -isystem/feather/contrib/depends/x86_64-apple-darwin/native/SDK/usr/include -Xclang -internal-externc-isystem/gnu/store/rwsysyzpxzwi7g1jv4hxwi3m8i36iwcc-clang-10.0.1/lib/clang/10.0.1/include -Xclang -internal-externc-isystem/feather/contrib/depends/x86_64-apple-darwin/native/SDK/usr/include")

set(CMAKE_INSTALL_NAME_TOOL x86_64-apple-darwin-install_name_tool)
set(CMAKE_FIND_ROOT_PATH /feather/contrib/depends/x86_64-apple-darwin/)

# Adjust the default behavior of the find commands:
# search headers and libraries in the target environment
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)

# Search programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
