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

target_link_libraries(log
    ${LIBS}
)

if(NOT BUILD_AS_PLUGIN)
    # Install headers to include directory
    foreach(HEADER ${LOG_HEADERS})
        get_filename_component(HEADER_NAME ${HEADER} NAME)
        configure_file(${HEADER} ${CMAKE_SOURCE_DIR}/app/include/log/${HEADER_NAME} COPYONLY)
    endforeach()
    
    message(STATUS "Installing log headers to ${CMAKE_SOURCE_DIR}/app/include/log")

    # Install library
    install(TARGETS log
        LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/app/lib
        ARCHIVE DESTINATION ${CMAKE_SOURCE_DIR}/app/lib
    )
endif()