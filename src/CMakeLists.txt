set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# pthread
find_package(Threads REQUIRED)

set(QT5_COMPONENTS
        Core
        Widgets
        Gui
        Network
        Svg
        Xml
        WebSockets
)

if (WITH_SCANNER)
    list(APPEND QT5_COMPONENTS
            Multimedia
            MultimediaWidgets)
endif()

find_package(Qt5 REQUIRED COMPONENTS ${QT5_COMPONENTS})

add_subdirectory(openpgp)

qt5_add_resources(RESOURCES assets.qrc assets_tor.qrc)

# Compile source files (.h/.cpp)
file(GLOB SOURCE_FILES
        "*.h"
        "*.cpp"
        "api/*.h"
        "api/*.cpp"
        "utils/*.h"
        "utils/*.cpp"
        "utils/os/*.h"
        "utils/os/*.cpp"
        "libwalletqt/*.h"
        "libwalletqt/*.cpp"
        "daemon/*.h"
        "daemon/*.cpp"
        "model/*.h"
        "model/*.cpp"
        "qt/*.h"
        "qt/*.cpp"
        "ui/qdarkstyle/style.qrc"
        "ui/BreezeStyleSheets/breeze.qrc"
        "widgets/*.h"
        "widgets/*.cpp"
        "wizard/*.h"
        "wizard/*.cpp"
        "wallet/*.h"
        "wallet/*.cpp"
        "qrcode/*.h"
        "qrcode/*.cpp"
        "dialog/*.h"
        "dialog/*.cpp"
        "qrcode_scanner/QrCodeUtils.cpp"
        "qrcode_scanner/QrCodeUtils.h"
        )

if (WITH_SCANNER)
    file(GLOB SCANNER_FILES
            "qrcode_scanner/*.h"
            "qrcode_scanner/*.cpp")

    list(APPEND SOURCE_FILES
            ${SCANNER_FILES})
endif()

set(EXECUTABLE_FLAG)
if(MINGW)
    set(EXECUTABLE_FLAG WIN32)

    set(ICON ${PROJECT_SOURCE_DIR}/src/assets/images/appicons/appicon.ico)
    set(ICON_RC ${CMAKE_CURRENT_BINARY_DIR}/icon.rc)
    set(ICON_RES ${CMAKE_CURRENT_BINARY_DIR}/icon.o)
    file(WRITE ${ICON_RC} "IDI_ICON1 ICON DISCARDABLE \"${ICON}\"")
    find_program(Qt5_WINDRES_EXECUTABLE NAMES windres x86_64-w64-mingw32-windres REQUIRED CMAKE_FIND_ROOT_PATH_BOTH)
    add_custom_command(OUTPUT ${ICON_RES} COMMAND ${Qt5_WINDRES_EXECUTABLE} ${ICON_RC} ${ICON_RES} MAIN_DEPENDENCY ${ICON_RC})
    list(APPEND RESOURCES ${ICON_RES})
endif()

# mac os bundle
if(APPLE)
    set(ICON ${PROJECT_SOURCE_DIR}/src/assets/images/appicons/appicon.icns)
    set_source_files_properties(${ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    list(APPEND RESOURCES ${ICON})
endif()

add_executable(feather ${EXECUTABLE_FLAG} main.cpp
        ${SOURCE_FILES}
        ${RESOURCES}
        ${ASSETS_TOR}
)

# mac os bundle
set_target_properties(feather PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/utils/Info.plist"
        LINK_FLAGS_RELEASE -s
)

set_property(TARGET feather PROPERTY RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

#target_include_directories(feather PUBLIC ${OPENGL_INCLUDE_DIR})
target_include_directories(feather PUBLIC ${Qt5Gui_PRIVATE_INCLUDE_DIRS})

target_include_directories(feather PUBLIC
        ${CMAKE_BINARY_DIR}/src/feather_autogen/include
        ${CMAKE_SOURCE_DIR}/monero/include
        ${CMAKE_SOURCE_DIR}/monero/src
        ${CMAKE_SOURCE_DIR}/monero/external/easylogging++
        ${CMAKE_SOURCE_DIR}/monero/contrib/epee/include
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/daemon
        ${CMAKE_CURRENT_SOURCE_DIR}/libwalletqt
        ${CMAKE_CURRENT_SOURCE_DIR}/model
        ${CMAKE_CURRENT_SOURCE_DIR}/utils
        ${CMAKE_CURRENT_SOURCE_DIR}/qrcode
        ${X11_INCLUDE_DIR}
        ${Boost_INCLUDE_DIRS}
        ${OPENSSL_INCLUDE_DIR}
        ${Qt5Core_INCLUDE_DIRS}
        ${Qt5Widgets_INCLUDE_DIRS}
        ${Qt5Gui_INCLUDE_DIRS}
        ${Qt5Network_INCLUDE_DIRS}
        ${Qt5Svg_INCLUDE_DIRS}
        ${Qt5Xml_INCLUDE_DIRS}
        ${Qt5WebSockets_INCLUDE_DIRS}
        ${ZLIB_INCLUDE_DIRS}
        ${LIBZIP_INCLUDE_DIRS}
        ${ZBAR_INCLUDE_DIR}
        )

if(WITH_SCANNER)
    target_include_directories(feather PUBLIC
            ${Qt5Multimedia_INCLUDE_DIRS}
            ${Qt5MultimediaWidgets_INCLUDE_DIRS}
    )
endif()

if(DONATE_BEG)
    target_compile_definitions(feather PRIVATE DONATE_BEG=1)
endif()

if (CHECK_UPDATES)
    target_compile_definitions(feather PRIVATE CHECK_UPDATES=1)
endif()

if(LOCALMONERO)
    target_compile_definitions(feather PRIVATE HAS_LOCALMONERO=1)
endif()

if(TOR_DIR)
    target_compile_definitions(feather PRIVATE HAS_TOR_BIN=1)
endif()

if(XMRIG)
    target_compile_definitions(feather PRIVATE HAS_XMRIG=1)
endif()

if(WITH_SCANNER)
    target_compile_definitions(feather PRIVATE WITH_SCANNER=1)
endif()

# TODO: PLACEHOLDER
target_compile_definitions(feather PRIVATE HAS_WEBSOCKET=1)

if(PLATFORM_INSTALLER)
    target_compile_definitions(feather PRIVATE PLATFORM_INSTALLER=1)
endif()

if(HAVE_SYS_PRCTL_H)
    target_compile_definitions(feather PRIVATE HAVE_SYS_PRCTL_H=1)
endif()

if(STATIC)
    target_compile_definitions(feather PRIVATE STATIC=1)
endif()

if("$ENV{DRONE}" STREQUAL "true")
    target_compile_definitions(feather PRIVATE DRONE=1)
endif()

if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(feather PRIVATE QT_NO_DEBUG=1)
endif()

target_compile_definitions(feather
        PUBLIC
        ${Qt5Core_DEFINITIONS}
        ${Qt5Widgets_DEFINITIONS}
        ${Qt5Gui_DEFINITIONS}
        ${Qt5Network_DEFINITIONS}
        ${Qt5Svg_DEFINITIONS}
        ${Qt5Xml_DEFINITIONS}
        ${Qt5WebSockets_DEFINITIONS}
)

if (WITH_SCANNER)
    target_compile_definitions(feather PUBLIC
            ${Qt5Multimedia_DEFINITIONS}
            ${Qt5MultimediaWidgets_DEFINITIONS}
    )
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

if(UNIX AND NOT APPLE)
    # https://stackoverflow.com/questions/57766620/cmake-add-library-doesnt-initialize-static-global-variable
    # so that contrib/monero-seed/src/gf_elem.cpp properly initializes. A better solution is welcome.
    target_link_libraries(feather -Wl,--whole-archive monero-seed -Wl,--no-whole-archive)
else()
    target_link_libraries(feather monero-seed)
endif()

target_link_libraries(feather
        wallet_merged
        ${LMDB_LIBRARY}
        epee
        ${UNBOUND_LIBRARY}
        ${SODIUM_LIBRARY}
        easylogging
        blockchain_db
        hardforks
        ${Boost_LIBRARIES}
        ${OPENSSL_LIBRARIES}
        ${CMAKE_DL_LIBS}
        ${EXTRA_LIBRARIES}
        Qt5::Core
        Qt5::Widgets
        Qt5::Gui
        Qt5::Network
        Qt5::Svg
        Qt5::Xml
        Qt5::WebSockets
        ${ICU_LIBRARIES}
        openpgp
        Threads::Threads
        ${QRENCODE_LIBRARY}
        ${ZLIB_LIBRARIES}
        ${LIBZIP_LIBRARIES}
        ${ZBAR_LIBRARIES}
        )

if(DEVICE_TREZOR_READY)
    target_link_libraries(feather ${TREZOR_DEP_LIBS})
endif()

if (WITH_SCANNER)
    target_link_libraries(feather
            Qt5::Multimedia
            Qt5::MultimediaWidgets
    )
endif()

if(NOT APPLE)
    target_link_libraries(feather
            Qt5::QSvgIconPlugin
            Qt5::QSvgPlugin
    )
endif()

if(STATIC)
    target_link_libraries(feather
            Qt5::QSvgIconPlugin
            Qt5::QSvgPlugin)
    if(UNIX AND NOT APPLE)
        target_link_libraries(feather
            Qt5::QXcbIntegrationPlugin)
    endif()
endif()

if(X11_FOUND)
    target_link_libraries(feather ${X11_LIBRARIES})
endif()

if(APPLE)
    include(Deploy)
endif()

install(TARGETS feather
        DESTINATION ${CMAKE_INSTALL_PREFIX}
)
