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

if(NOT BUILD_AS_PLUGIN)
    # Install headers to include directory
    foreach(HEADER ${CORE_HEADERS})
        get_filename_component(HEADER_NAME ${HEADER} NAME)
        configure_file(${HEADER} ${CMAKE_SOURCE_DIR}/app/include/core/${HEADER_NAME} COPYONLY)
    endforeach()
    
    message(STATUS "Installing core headers to ${CMAKE_SOURCE_DIR}/app/include/core")

    # Install library
    install(TARGETS core
        LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/app/lib
        ARCHIVE DESTINATION ${CMAKE_SOURCE_DIR}/app/lib
    )
endif()