file(GLOB OPENGL_SOURCES
    "*.cpp"
)

file(GLOB OPENGL_HEADERS
    "*.h"
)

find_package(Qt5 COMPONENTS Core Widgets Gui OpenGL REQUIRED)

add_library(opengl SHARED
    ${OPENGL_SOURCES}
    ${OPENGL_HEADERS}
)

target_include_directories(opengl PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${INCLUDES}
)

target_link_libraries(opengl
    ${LIBS}
    Qt5::OpenGL
)