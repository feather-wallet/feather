if(APPLE OR (WIN32 AND NOT STATIC))
    add_custom_target(deploy)
    get_target_property(_qmake_executable Qt::qmake IMPORTED_LOCATION)
    get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)

    if(APPLE AND NOT IOS)
        find_program(MACDEPLOYQT_EXECUTABLE macdeployqt HINTS "${_qt_bin_dir}")
        MESSAGE(INFO "MACDEPLOY ${MACDEPLOYQT_EXECUTABLE}")
        add_custom_command(TARGET deploy
                POST_BUILD
                COMMAND "${MACDEPLOYQT_EXECUTABLE}" "$<TARGET_FILE_DIR:feather>/../.." -always-overwrite
                COMMENT "Running macdeployqt..."
                )

        # workaround for a Qt bug that requires manually adding libqsvg.dylib to bundle
        find_file(_qt_svg_dylib "libqsvg.dylib" PATHS "${CMAKE_PREFIX_PATH}/plugins/imageformats" NO_DEFAULT_PATH)
        if(_qt_svg_dylib)
            add_custom_command(TARGET deploy
                    POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy ${_qt_svg_dylib} $<TARGET_FILE_DIR:feather>/../PlugIns/imageformats/
                    COMMAND ${CMAKE_INSTALL_NAME_TOOL} -change "${CMAKE_PREFIX_PATH}/lib/QtGui.framework/Versions/5/QtGui" "@executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui" $<TARGET_FILE_DIR:feather>/../PlugIns/imageformats/libqsvg.dylib
                    COMMAND ${CMAKE_INSTALL_NAME_TOOL} -change "${CMAKE_PREFIX_PATH}/lib/QtWidgets.framework/Versions/5/QtWidgets" "@executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui" $<TARGET_FILE_DIR:feather>/../PlugIns/imageformats/libqsvg.dylib
                    COMMAND ${CMAKE_INSTALL_NAME_TOOL} -change "${CMAKE_PREFIX_PATH}/lib/QtSvg.framework/Versions/5/QtSvg" "@executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui" $<TARGET_FILE_DIR:feather>/../PlugIns/imageformats/libqsvg.dylib
                    COMMAND ${CMAKE_INSTALL_NAME_TOOL} -change "${CMAKE_PREFIX_PATH}/lib/QtCore.framework/Versions/5/QtCore" "@executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui" $<TARGET_FILE_DIR:feather>/../PlugIns/imageformats/libqsvg.dylib
                    COMMENT "Copying libqsvg.dylib, running install_name_tool"
                    )
        endif()
    endif()
endif()