cmake_minimum_required(VERSION 3.16)

project(
  3Dschool
  VERSION 0.1
  DESCRIPTION "3D school airspace simulation"
  LANGUAGES CXX)

# Add build option
option(BUILD_AS_PLUGIN "Build as QGIS plugin" OFF)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt-specific extensions
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(QGIS REQUIRED)
find_package(Qt5 COMPONENTS Core Widgets Xml Gui WebEngineWidgets REQUIRED)

set(LIBS
  ${QGIS_CORE_LIBRARY}
  ${QGIS_GUI_LIBRARY}
  ${QGIS_ANALYSIS_LIBRARY}
  Qt5::Core
  Qt5::Gui
  Qt5::Xml
  Qt5::Widgets
  Qt5::WebEngineWidgets
)

set(INCLUDES
  ${Boost_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${QGIS_INCLUDE_DIR}
)

include_directories(
  ${INCLUDES}
)

# The compiled library code is here
add_subdirectory(src)