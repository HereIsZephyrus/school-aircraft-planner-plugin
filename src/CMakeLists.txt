
set(AVATAR_RESOURCES
    ${CMAKE_SOURCE_DIR}/resources/avatar.qrc
)

add_library(3Dschool MODULE
  qgis_plugin_main.cpp
  ${AVATAR_RESOURCES}
)

target_link_libraries(3Dschool
  ${LIBS}
  schoolcore
)

target_include_directories(3Dschool PUBLIC
  ${INCLUDES}
)

add_subdirectory(schoolcore)
