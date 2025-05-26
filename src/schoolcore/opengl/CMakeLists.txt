file(GLOB OPENGL_SOURCES
    "*.cpp"
)

file(GLOB OPENGL_HEADERS
    "*.h"
)

set(OPENGL_RESOURCES
    ${CMAKE_SOURCE_DIR}/resources/shader.qrc
)

find_package(Qt5 COMPONENTS Core Widgets Gui OpenGL REQUIRED)

add_library(opengl STATIC
    ${OPENGL_SOURCES}
    ${OPENGL_HEADERS}
    ${OPENGL_RESOURCES}
)

target_include_directories(opengl PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${INCLUDES}
)

target_link_libraries(opengl
    ${LIBS}
)