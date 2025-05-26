file(GLOB OPENGL_SOURCES
    "*.cpp"
)

file(GLOB OPENGL_HEADERS
    "*.h"
)

add_library(opengl STATIC
    ${OPENGL_SOURCES}
    ${OPENGL_HEADERS}
)

target_include_directories(opengl PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${INCLUDES}
)

target_link_libraries(core
    ${LIBS}
)