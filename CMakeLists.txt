cmake_minimum_required(VERSION 3.16)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

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

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

find_package(QGIS REQUIRED)
find_package(OpenGL REQUIRED)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets Xml Gui OpenGL REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Xml Gui OpenGL REQUIRED)
message(STATUS "Qt5Core_DIR: ${Qt5Core_DIR}")
message(STATUS "Qt5Core_LIBRARIES: ${Qt5Core_LIBRARIES}")

if(WIN32)
  set(GL_LIBS opengl32)
else()
  set(GL_LIBS GL)
endif()

set(LIBS
  ${QGIS_CORE_LIBRARY}
  ${QGIS_GUI_LIBRARY}
  ${QGIS_ANALYSIS_LIBRARY}
  Qt5::Core
  Qt5::Gui
  Qt5::Xml
  Qt5::Widgets
  Qt5::OpenGL
  ${GL_LIBS}
)

set(INCLUDES
  ${Boost_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${QGIS_INCLUDE_DIR}
)

set(DOC_RESOURCES
  ${CMAKE_SOURCE_DIR}/doc/documents.qrc
)

include_directories(
  ${Qt5Core_INCLUDE_DIRS}
  ${Qt5Gui_INCLUDE_DIRS}
  ${Qt5Xml_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}
  ${INCLUDES}
)

add_subdirectory(src)
