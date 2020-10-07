# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2020, The Monero Project.

find_package(Git QUIET)

# Check what commit we're on
execute_process(COMMAND "${GIT_EXECUTABLE}" rev-parse --short=9 HEAD RESULT_VARIABLE RET OUTPUT_VARIABLE COMMIT OUTPUT_STRIP_TRAILING_WHITESPACE)

if(RET)
    # Something went wrong, set the version tag to -unknown

    message(WARNING "Cannot determine current commit. Make sure that you are building either from a Git working tree or from a source archive.")
    set(FEATHER_BRANCH "unknown")
    configure_file("cmake/config-feather.h.cmake" "${CMAKE_CURRENT_SOURCE_DIR}/src/config-feather.h")
else()
    string(SUBSTRING ${COMMIT} 0 9 COMMIT)
    message(STATUS "You are currently on commit ${COMMIT}")

    # Get all the tags
    execute_process(COMMAND "${GIT}" rev-list --tags --max-count=1 --abbrev-commit RESULT_VARIABLE RET OUTPUT_VARIABLE TAGGEDCOMMIT OUTPUT_STRIP_TRAILING_WHITESPACE)

    if(NOT TAGGEDCOMMIT)
        message(STATUS "Cannot determine most recent tag. Make sure that you are building either from a Git working tree or from a source archive.")
        set(FEATHER_BRANCH "${COMMIT}")
    else()
        message(STATUS "The most recent tag was at ${TAGGEDCOMMIT}")

        # Check if we're building that tagged commit or a different one
        if(COMMIT STREQUAL TAGGEDCOMMIT)
            message(STATUS "You are building a tagged release")
            set(FEATHER_BRANCH "release")
        else()
            message(STATUS "You are ahead of or behind a tagged release")
            set(FEATHER_BRANCH "${COMMIT}")
        endif()
    endif()

    configure_file("cmake/config-feather.h.cmake" "${CMAKE_CURRENT_SOURCE_DIR}/src/config-feather.h")
endif()
