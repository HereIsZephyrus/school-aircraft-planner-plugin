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

if(NOT BUILD_AS_PLUGIN)
    # Install headers to include directory
    foreach(HEADER ${GUI_HEADERS})
        get_filename_component(HEADER_NAME ${HEADER} NAME)
        configure_file(${HEADER} ${CMAKE_SOURCE_DIR}/app/include/gui/${HEADER_NAME} COPYONLY)
    endforeach()
    
    message(STATUS "Installing gui headers to ${CMAKE_SOURCE_DIR}/app/include/gui")

    # Install library
    install(TARGETS gui
        LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/app/lib
        ARCHIVE DESTINATION ${CMAKE_SOURCE_DIR}/app/lib
    )
endif()