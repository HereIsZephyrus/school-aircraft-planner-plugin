file(GLOB CORE_SOURCES
    "*.cpp"
)

file(GLOB CORE_HEADERS
    "*.h"
)

add_library(core SHARED
    ${CORE_SOURCES}
    ${CORE_HEADERS}
)

target_include_directories(core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${INCLUDES}
)

target_link_libraries(core
    ${LIBS}
)