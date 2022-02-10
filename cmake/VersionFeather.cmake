# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2020-2022 The Monero Project

find_package(Git QUIET)

# Sets FEATHER_COMMIT to the first 9 chars of the current commit hash.

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/githash.txt")
    # This file added in source archives where the .git folder has been removed to optimize for space.
    file(READ "githash.txt" COMMIT)
    string(SUBSTRING ${COMMIT} 0 9 COMMIT)
    message(STATUS "You are currently on commit ${COMMIT}")
    set(FEATHER_COMMIT "${COMMIT}")
else()
    execute_process(COMMAND "${GIT_EXECUTABLE}" rev-parse --short=9 HEAD RESULT_VARIABLE RET OUTPUT_VARIABLE COMMIT OUTPUT_STRIP_TRAILING_WHITESPACE)

    if(RET)
        message(WARNING "Cannot determine current commit. Make sure that you are building either from a Git working tree or from a source archive.")
        set(FEATHER_COMMIT "unknown")
    else()
        string(SUBSTRING ${COMMIT} 0 9 COMMIT)
        message(STATUS "You are currently on commit ${COMMIT}")
        set(FEATHER_COMMIT "${COMMIT}")
    endif()
endif()

configure_file("cmake/config-feather.h.cmake" "${CMAKE_CURRENT_SOURCE_DIR}/src/config-feather.h")