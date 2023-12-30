find_path(BCUR_INCLUDE_DIR "bcur/bc-ur.hpp")
find_library(BCUR_LIBRARY bcur)

if (NOT BCUR_INCLUDE_DIR OR NOT BCUR_LIBRARY)
    MESSAGE(STATUS "Could not find installed BCUR, using vendored library instead")
    set(BCUR_VENDORED "ON")
    set(BCUR_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/src/third-party)
    set(BCUR_LIBRARY bcur_static)
endif()

message(STATUS "BCUR PATH ${BCUR_INCLUDE_DIR}")
message(STATUS "BCUR LIBRARY ${BCUR_LIBRARY}")