file(GLOB LOG_SOURCES
    "*.cpp"
)

file(GLOB LOG_HEADERS
    "*.h"
)

add_library(log STATIC
    ${LOG_SOURCES}
    ${LOG_HEADERS}
)

target_include_directories(log PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${INCLUDES}
)

find_package(QGIS REQUIRED)

target_link_libraries(log
    ${LIBS}
)