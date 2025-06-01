# Set RPATH settings based on build type
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_SKIP_BUILD_RPATH FALSE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

    if(APPLE)
        set(CMAKE_INSTALL_RPATH "@executable_path/../lib")
    else()
        set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
    endif()
else()
    # Debug build - use system libraries directly
    set(CMAKE_SKIP_BUILD_RPATH FALSE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
    if(APPLE)
        set(CMAKE_INSTALL_RPATH "@executable_path/../lib")
    else()
        set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
    endif()
endif()

set(RESOURCES
    ${CMAKE_SOURCE_DIR}/resources/resources.qrc
    ${CMAKE_SOURCE_DIR}/resources/avatar.qrc
    ${CMAKE_SOURCE_DIR}/resources/shader.qrc
    ${DOC_RESOURCES}
)

set(CORE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/core/RoutePlanner.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/RoutePlanner.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/WorkspaceState.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/WorkspaceState.h
)

set(GUI_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/gui/Canvas.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gui/Canvas.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gui/LeftDockWidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gui/LeftDockWidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gui/RightDockWidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gui/RightDockWidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gui/Menu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gui/Menu.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gui/OpenGLCanvas.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gui/OpenGLCanvas.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gui/StyleManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gui/StyleManager.h
)

set(LOG_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/log/QgisDebug.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/log/QgisDebug.h
)

set(OPENGL_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/opengl/Camera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/opengl/Primitive.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/opengl/Camera.h
    ${CMAKE_CURRENT_SOURCE_DIR}/opengl/Primitive.h
)

set(SCHOOLCORE_SOUCRES
    ${LOG_SOURCES}
    ${OPENGL_SOURCES}
    ${CORE_SOURCES}
    ${GUI_SOURCES}
)

set(MAIN_WINDOW_SOURCES
    MainWindow.cpp
    MainWindow.h
    MainWindow.ui
)

set(STANDALONE_SOURCES
    main.cpp
    ${MAIN_WINDOW_SOURCES}
)

set(PLUGIN_SOURCES
    plugin.cpp
    plugin.h
    ${MAIN_WINDOW_SOURCES}
)

if(BUILD_AS_PLUGIN)
    # Build as QGIS plugin
    add_library(3Dschool MODULE
        ${PLUGIN_SOURCES}
        ${SCHOOLCORE_SOUCRES}
        ${RESOURCES}
    )

    target_link_libraries(3Dschool
        ${LIBS}
    )

    target_include_directories(3Dschool PUBLIC
        ${INCLUDES}
    )
else()

    if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
        qt_add_executable(3Dschool
            MANUAL_FINALIZATION
            ${SCHOOLCORE_SOUCRES}
            ${STANDALONE_SOURCES}
            ${RESOURCES}
        )
    # Define target properties for Android with Qt 6 as:
    #    set_property(TARGET 3Dschool APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
    #                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
    # For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
    else()
        if(ANDROID)
            add_library(3Dschool SHARED
                ${SCHOOLCORE_SOUCRES}
                ${STANDALONE_SOURCES}
                ${RESOURCES}
            )
    # Define properties for Android with Qt 5 after find_package() calls as:
    #    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
        else()
            add_executable(3Dschool
                ${SCHOOLCORE_SOUCRES}
                ${STANDALONE_SOURCES}
                ${RESOURCES}
            )
        endif()
    endif()

    target_link_libraries(3Dschool PRIVATE ${LIBS})

    # Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
    # If you are developing for iOS or macOS you should consider setting an
    # explicit, fixed bundle identifier manually though.
    if(${QT_VERSION} VERSION_LESS 6.1.0)
      set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.3Dschool)
    endif()
    set_target_properties(3Dschool PROPERTIES
        ${BUNDLE_ID_OPTION}
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE TRUE
    )

    include(GNUInstallDirs)
    install(TARGETS 3Dschool
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    if(QT_VERSION_MAJOR EQUAL 6)
        qt_finalize_executable(3Dschool)
    endif()
endif()
