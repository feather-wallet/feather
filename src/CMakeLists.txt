set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# pthread
find_package(Threads REQUIRED)

set(QT_COMPONENTS
        Core
        Widgets
        Gui
        Network
        Svg
        Xml
        WebSockets
)

if (WITH_SCANNER)
    list(APPEND QT_COMPONENTS
            Multimedia
            MultimediaWidgets)
endif()

find_package(Qt6 REQUIRED COMPONENTS ${QT_COMPONENTS} SvgWidgets)
if (NOT Qt6_FOUND)
    find_package(Qt5 5.15 REQUIRED COMPONENTS ${QT_COMPONENTS})
endif()

set(QAPPLICATION_CLASS QApplication CACHE STRING "Inheritance class for SingleApplication")
add_subdirectory(third-party/singleapplication)

add_subdirectory(openpgp)

qt_add_resources(RESOURCES assets.qrc assets_tor.qrc)

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
        "polyseed/*.h"
        "polyseed/*.cpp"
        "polyseed/*.c"
        "qrcode_scanner/QrCodeUtils.cpp"
        "qrcode_scanner/QrCodeUtils.h"
        )

if (WITH_SCANNER AND NOT Qt6_FOUND)
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
    find_program(Qt_WINDRES_EXECUTABLE NAMES windres x86_64-w64-mingw32-windres REQUIRED CMAKE_FIND_ROOT_PATH_BOTH)
    add_custom_command(OUTPUT ${ICON_RES} COMMAND ${Qt_WINDRES_EXECUTABLE} ${ICON_RC} ${ICON_RES} MAIN_DEPENDENCY ${ICON_RC})
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

target_include_directories(feather PUBLIC ${QtGui_PRIVATE_INCLUDE_DIRS})

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
        ${QtCore_INCLUDE_DIRS}
        ${QtWidgets_INCLUDE_DIRS}
        ${QtGui_INCLUDE_DIRS}
        ${QtNetwork_INCLUDE_DIRS}
        ${QtSvg_INCLUDE_DIRS}
        ${QtSvgWidgets_INCLUDE_DIRS}
        ${QtXml_INCLUDE_DIRS}
        ${QtWebSockets_INCLUDE_DIRS}
        ${ZLIB_INCLUDE_DIRS}
        ${LIBZIP_INCLUDE_DIRS}
        ${ZBAR_INCLUDE_DIR}
        ${POLYSEED_INCLUDE_DIR}
        )

if(WITH_SCANNER)
    target_include_directories(feather PUBLIC
            ${QtMultimedia_INCLUDE_DIRS}
            ${QtMultimediaWidgets_INCLUDE_DIRS}
    )
endif()

if(SELF_CONTAINED)
    target_compile_definitions(feather PRIVATE SELF_CONTAINED=1)
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

if(WITH_SCANNER AND NOT Qt6_FOUND)
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
        ${QtCore_DEFINITIONS}
        ${QtWidgets_DEFINITIONS}
        ${QtGui_DEFINITIONS}
        ${QtNetwork_DEFINITIONS}
        ${QtSvg_DEFINITIONS}
        ${QtSvgWidgets_DEFINITIONS}
        ${QtXml_DEFINITIONS}
        ${QtWebSockets_DEFINITIONS}
)

if (WITH_SCANNER)
    target_compile_definitions(feather PUBLIC
            ${QtMultimedia_DEFINITIONS}
            ${QtMultimediaWidgets_DEFINITIONS}
    )
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${QtWidgets_EXECUTABLE_COMPILE_FLAGS}")

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
        Qt::Core
        Qt::Widgets
        Qt::Gui
        Qt::Network
        Qt::Svg
        Qt::SvgWidgets
        Qt::Xml
        Qt::WebSockets
        ${ICU_LIBRARIES}
        openpgp
        Threads::Threads
        ${QRENCODE_LIBRARY}
        ${ZLIB_LIBRARIES}
        ${LIBZIP_LIBRARIES}
        ${ZBAR_LIBRARIES}
        ${POLYSEED_LIBRARY}
        SingleApplication::SingleApplication
        )

if(DEVICE_TREZOR_READY)
    target_link_libraries(feather ${TREZOR_DEP_LIBS})
endif()

if (WITH_SCANNER)
    target_link_libraries(feather
            Qt::Multimedia
            Qt::MultimediaWidgets
    )
endif()

if(NOT APPLE AND NOT Qt6_FOUND)
    target_link_libraries(feather
            Qt::QSvgIconPlugin
            Qt::QSvgPlugin
    )
endif()

if(STATIC)
    if(NOT Qt6_FOUND)
    target_link_libraries(feather
            Qt::QSvgIconPlugin
            Qt::QSvgPlugin)
    endif()
    if(UNIX AND NOT APPLE)
        target_link_libraries(feather
            Qt::QXcbIntegrationPlugin)
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
