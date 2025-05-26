add_library(3Dschool MODULE
  qgis_plugin_main.cpp
)

target_link_libraries(3Dschool
  ${LIBS}
  schoolcore
)

target_include_directories(3Dschool PUBLIC
  ${INCLUDES}
)

add_subdirectory(schoolcore)
