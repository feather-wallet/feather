message(STATUS "Generating docs")

find_package(Python3 COMPONENTS Interpreter)

if(Python3_Interpreter_FOUND)
    execute_process(COMMAND python3 contrib/docs/generate.py
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

    FILE(GLOB DOCS LIST_DIRECTORIES false "src/assets/docs/*")

    foreach(FILE ${DOCS})
        cmake_path(GET FILE FILENAME FILE_REL)
        list(APPEND QRC_LIST "        <file alias=\"${FILE_REL}\">${FILE}</file>")
    endforeach()

    list(JOIN QRC_LIST "\n" QRC_DATA)
    configure_file("cmake/assets_docs.qrc" "${CMAKE_CURRENT_SOURCE_DIR}/src/assets_docs.qrc")
else()
    message(WARNING "No Python3 interpreter, skipping docs.")
endif()
