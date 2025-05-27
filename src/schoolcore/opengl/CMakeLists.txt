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
)

if(NOT BUILD_AS_PLUGIN)
    # Install headers to include directory
    foreach(HEADER ${OPENGL_HEADERS})
        get_filename_component(HEADER_NAME ${HEADER} NAME)
        configure_file(${HEADER} ${CMAKE_SOURCE_DIR}/app/include/opengl/${HEADER_NAME} COPYONLY)
    endforeach()
    
    message(STATUS "Installing opengl headers to ${CMAKE_SOURCE_DIR}/app/include/opengl")

    # Install library
    install(TARGETS opengl
        LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/app/lib
        ARCHIVE DESTINATION ${CMAKE_SOURCE_DIR}/app/lib
    )
endif()