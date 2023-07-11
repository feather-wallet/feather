cmake_minimum_required(VERSION 3.5)

project(quirc)

add_library(quirc STATIC
        lib/decode.c
        lib/identify.c
        lib/quirc.c
        lib/version_db.c
        )

target_include_directories(quirc PUBLIC lib)

install(TARGETS quirc)

install(FILES
        lib/quirc.h
        DESTINATION include/quirc)
