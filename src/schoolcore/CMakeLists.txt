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

add_subdirectory(core)
add_subdirectory(gui)
add_subdirectory(log)
add_subdirectory(opengl)