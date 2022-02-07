# Copyright (c) 2020, tevador <tevador@gmail.com>

cmake_minimum_required(VERSION 3.13)

option(MONERO_SEED_DEMO "Build a demo executable for monero-seed")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
  message(STATUS "Setting default build type: ${CMAKE_BUILD_TYPE}")
endif()

project(monero-seed)

include(GNUInstallDirs)

add_library(${PROJECT_NAME}
src/argon2/blake2/blake2b.c
src/argon2/argon2.c
src/argon2/core.c
src/argon2/ref.c
src/galois_field.cpp
src/gf_elem.cpp
src/gf_poly.cpp
src/monero_seed.cpp
src/pbkdf2.c
src/reed_solomon_code.cpp
src/secure_random.cpp
src/wordlist.cpp)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)
target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/monero_seed>)

if(MONERO_SEED_DEMO)
  add_executable(demo src/main.cpp)
  set_property(TARGET demo PROPERTY CXX_STANDARD 11)
  target_link_libraries(demo -Wl,--whole-archive ${PROJECT_NAME} -Wl,--no-whole-archive)
endif()

install(TARGETS ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT ${PROJECT_NAME}
  FILE ${PROJECT_NAME}Config.cmake
  NAMESPACE ${PROJECT_NAME}::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})
