# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/app/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/app/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/app/lib)

# Create necessary directories
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/app/bin)
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/app/lib)
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/app/include)

# Set RPATH settings
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

if(APPLE)
    set(CMAKE_INSTALL_RPATH "@executable_path/../lib")
else()
    set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
endif()

set(AVATAR_RESOURCES
    ${CMAKE_SOURCE_DIR}/resources/avatar.qrc
)

# Add schoolcore library first
add_subdirectory(schoolcore)

if(BUILD_AS_PLUGIN)
    # Build as QGIS plugin
    add_library(3Dschool MODULE
        qgis_plugin_main.cpp
        ${AVATAR_RESOURCES}
    )

    target_link_libraries(3Dschool
        ${LIBS}
        schoolcore
    )

    target_include_directories(3Dschool PUBLIC
        ${INCLUDES}
    )
else()
    # Build as standalone application
    add_executable(3Dschool_app
        main.cpp
        ${AVATAR_RESOURCES}
    )

    if(NOT APPLE)
        set_target_properties(3Dschool_app PROPERTIES
            INSTALL_RPATH "$ORIGIN/../lib"
            BUILD_WITH_INSTALL_RPATH TRUE
            LINK_FLAGS "-Wl,--disable-new-dtags,-rpath,$ORIGIN/../lib"
        )
    endif()

    # Link against Qt libraries explicitly
    target_link_libraries(3Dschool_app PRIVATE
        Qt5::Core
        Qt5::Widgets
        Qt5::Gui
        schoolcore
        ${LIBS}
    )

    target_include_directories(3Dschool_app PUBLIC
        ${INCLUDES}
        ${CMAKE_SOURCE_DIR}/src/schoolcore
    )

    # Install the executable
    install(TARGETS 3Dschool_app
        RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/app/bin
    )

    # Create copy_deps script
    file(WRITE ${CMAKE_BINARY_DIR}/copy_deps.sh
"#!/bin/bash
echo 'Copying system dependencies...'
TARGET=\"$1\"
DEST=\"$2\"
ldd \"$TARGET\" | grep '=>' | awk '{print $3}' | while read -r lib; do
    if [ -f \"$lib\" ]; then
        cp -v \"$lib\" \"$DEST/\"
    fi
done
")

    # Make copy_deps script executable
    file(CHMOD ${CMAKE_BINARY_DIR}/copy_deps.sh
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    )

    # Add custom command to copy dependencies after build
    add_custom_command(TARGET 3Dschool_app POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Copying system dependencies..."
        COMMAND bash ${CMAKE_BINARY_DIR}/copy_deps.sh $<TARGET_FILE:3Dschool_app> ${CMAKE_SOURCE_DIR}/app/lib
    )

    # For Qt dependencies
    if(Qt5_FOUND)
        # Create plugins directory
        file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/app/bin/platforms)
        file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/app/bin/imageformats)
        file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/app/bin/iconengines)

        # Copy Qt libraries
        foreach(QtModule Core Widgets Gui DBus XcbQpa)
            if(TARGET Qt5::${QtModule})
                get_target_property(QT_LIB_PATH Qt5::${QtModule} LOCATION)
                if(QT_LIB_PATH)
                    add_custom_command(TARGET 3Dschool_app POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                            ${QT_LIB_PATH}
                            ${CMAKE_SOURCE_DIR}/app/lib/
                    )
                endif()
            endif()
        endforeach()

        # Get Qt plugins directory
        get_target_property(QT_QPA_PLUGIN_PATH Qt5::QXcbIntegrationPlugin LOCATION)
        if(QT_QPA_PLUGIN_PATH)
            get_filename_component(QT_PLUGIN_DIR ${QT_QPA_PLUGIN_PATH} DIRECTORY)
            get_filename_component(QT_PLUGIN_DIR ${QT_PLUGIN_DIR}/.. ABSOLUTE)
        else()
            set(QT_PLUGIN_DIR "${_qt5Core_install_prefix}/plugins")
        endif()

        # Install Qt plugins
        set(QT_PLUGIN_DIRS
            ${QT_PLUGIN_DIR}/platforms
            ${QT_PLUGIN_DIR}/imageformats
            ${QT_PLUGIN_DIR}/iconengines
            ${QT_PLUGIN_DIR}/xcbglintegrations
        )
        
        foreach(PLUGIN_DIR ${QT_PLUGIN_DIRS})
            if(EXISTS ${PLUGIN_DIR})
                file(GLOB PLUGIN_FILES "${PLUGIN_DIR}/*")
                foreach(PLUGIN ${PLUGIN_FILES})
                    if(PLUGIN MATCHES ".*\\.so$")
                        get_filename_component(PLUGIN_NAME ${PLUGIN} NAME)
                        get_filename_component(PLUGIN_DIR_NAME ${PLUGIN_DIR} NAME)
                        add_custom_command(TARGET 3Dschool_app POST_BUILD
                            COMMAND ${CMAKE_COMMAND} -E make_directory
                                ${CMAKE_SOURCE_DIR}/app/bin/${PLUGIN_DIR_NAME}
                            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                                ${PLUGIN}
                                ${CMAKE_SOURCE_DIR}/app/bin/${PLUGIN_DIR_NAME}/
                        )
                    endif()
                endforeach()
            endif()
        endforeach()

        # Create qt.conf with correct plugin path
        file(WRITE ${CMAKE_SOURCE_DIR}/app/bin/qt.conf
"[Paths]
Plugins = .
Libraries = ../lib
")
    endif()

    # Create run script with proper environment setup
    file(WRITE ${CMAKE_SOURCE_DIR}/app/bin/run.sh
"#!/bin/bash
SCRIPT_DIR=\"\$(cd \"\$(dirname \"\${BASH_SOURCE[0]}\")\" && pwd)\"
export LD_LIBRARY_PATH=\"\${SCRIPT_DIR}/../lib:\${LD_LIBRARY_PATH}\"
export QT_PLUGIN_PATH=\"\${SCRIPT_DIR}\"
export QT_DEBUG_PLUGINS=1
\${SCRIPT_DIR}/3Dschool_app \"$@\"
")
    
    # Make run script executable
    file(CHMOD ${CMAKE_SOURCE_DIR}/app/bin/run.sh
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    )
endif()
