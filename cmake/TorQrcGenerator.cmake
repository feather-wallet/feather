if (TOR_DIR)
    FILE(GLOB TOR_FILES LIST_DIRECTORIES false ${TOR_DIR}/*)

    foreach(FILE ${TOR_FILES})
        cmake_path(GET FILE FILENAME FILE_REL)
        list(APPEND QRC_LIST "        <file>assets/tor/${FILE_REL}</file>")

        if (FILE_REL STREQUAL "tor" OR FILE_REL STREQUAL "tor.exe")
            set(TOR_BIN_FOUND 1)
        endif()
    endforeach()

    if (NOT TOR_BIN_FOUND)
        message(FATAL_ERROR "TOR_DIR was specified but the Tor binary could not be found")
    endif()
endif()

list(JOIN QRC_LIST "\n" QRC_DATA)
configure_file("cmake/assets_tor.qrc" "${CMAKE_CURRENT_SOURCE_DIR}/src/assets_tor.qrc")