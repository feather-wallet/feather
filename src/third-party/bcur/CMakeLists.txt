cmake_minimum_required(VERSION 3.5)

project(bcur)

set(bcur_sources
        bytewords.cpp
        fountain-encoder.cpp
        fountain-decoder.cpp
        fountain-utils.cpp
        xoshiro256.cpp
        utils.cpp
        random-sampler.cpp
        ur-decoder.cpp
        ur.cpp
        ur-encoder.cpp
        memzero.c
        crc32.c
        sha2.c)

add_library(bcur_static STATIC ${bcur_sources})
set_property(TARGET bcur_static PROPERTY POSITION_INDEPENDENT_CODE ON)
