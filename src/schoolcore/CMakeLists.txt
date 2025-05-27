# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/app/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/app/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/app/lib)

# Create necessary directories
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/app/bin)
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/app/lib)
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/app/include)
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/app/include/core)
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/app/include/gui)
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/app/include/log)
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/app/include/opengl)

file(GLOB MAINWINDOW_SOURCES
    "MainWindow.cpp"
)

file(GLOB MAINWINDOW_HEADERS
    "MainWindow.h"
)

set(UI_FILES
    MainWindow.ui
)

set(RESOURCES_FILE
    ${CMAKE_SOURCE_DIR}/resources/resources.qrc
    ${CMAKE_SOURCE_DIR}/resources/shader.qrc
)

qt5_add_resources(RESOURCE_SOURCES
    ${RESOURCES_FILE}
)

add_library(schoolcore SHARED
    ${MAINWINDOW_SOURCES}
    ${MAINWINDOW_HEADERS}
    ${UI_FILES}
    ${RESOURCE_SOURCES}
)

target_include_directories(schoolcore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/gui
    ${CMAKE_CURRENT_SOURCE_DIR}/opengl
    ${CMAKE_CURRENT_SOURCE_DIR}/log
    ${INCLUDES}
)

target_link_libraries(schoolcore
    ${LIBS}
    core
    gui
    log
    opengl
)

if(NOT BUILD_AS_PLUGIN)
    # Install headers to include directory
    install(FILES ${MAINWINDOW_HEADERS}
        DESTINATION ${CMAKE_SOURCE_DIR}/app/include
    )
    message(STATUS "Installing headers to ${CMAKE_SOURCE_DIR}/app/include")

    # Install library
    install(TARGETS schoolcore
        LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/app/lib
        ARCHIVE DESTINATION ${CMAKE_SOURCE_DIR}/app/lib
    )
    message(STATUS "Installing library to ${CMAKE_SOURCE_DIR}/app/lib")
endif()

add_subdirectory(core)
add_subdirectory(gui)
add_subdirectory(log)
add_subdirectory(opengl)