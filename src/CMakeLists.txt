# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/app/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/app/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/app/lib)

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

    target_link_libraries(3Dschool_app
        ${LIBS}
        schoolcore
    )

    target_include_directories(3Dschool_app PUBLIC
        ${INCLUDES}
    )
endif()
