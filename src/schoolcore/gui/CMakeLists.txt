file(GLOB GUI_SOURCES
    "*.cpp"
)

file(GLOB GUI_HEADERS
    "*.h"
)

add_library(gui SHARED
    ${GUI_SOURCES}
    ${GUI_HEADERS}
)

target_include_directories(gui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${INCLUDES}
)

find_package(Qt5 COMPONENTS Core Widgets Gui OpenGL REQUIRED)

target_link_libraries(gui
    ${LIBS}
)