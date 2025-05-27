# # Once run this will define:
# #
# # QGIS_FOUND            = system has QGIS lib
# #
# # QGIS_CORE_LIBRARY     = full path to the CORE library
# # QGIS_GUI_LIBRARY      = full path to the GUI library
# # QGIS_ANALYSIS_LIBRARY = full path to the ANALYSIS library
# # QGIS_PLUGIN_DIR       = full path to where QGIS plugins are installed
# # QGIS_INCLUDE_DIR      = where to find headers
# # QGIS_UI_INCLUDE_DIR   = where to find ui_* generated headers
# #
# # QGIS_VERSION          = version as defined in qgsconfig.h, as major.minor.patch
# #
# # Definitions or ENV variables affecting search locations
# #
# # OSGEO4W_ROOT          = [A-Z]:/path/to/OSGeo4W/install/root
# #                               (^ use forward slashes!)
# # OSGEO4W_QGIS_SUBDIR   = qgis[-rel|-ltr][-dev], in OSGEO4W_ROOT/apps/
# # QGIS_MAC_PATH         = /path/to/any/QGIS.app/Contents
# # QGIS_BUILD_PATH       = [A-Z:]/path/to/QGIS/build/directory
# #
# # Tim Sutton + ChanningTong

# Set local QGIS path
set(LOCAL_QGIS_PATH "${CMAKE_SOURCE_DIR}/dependencies/qgis/")
find_path(QGIS_INCLUDE_DIR
    NAMES qgis.h
    PATHS
    ${LOCAL_QGIS_PATH}/include/qgis
    NO_DEFAULT_PATH
)
find_library(QGIS_CORE_LIBRARY
    NAMES qgis_core
    PATHS
    ${LOCAL_QGIS_PATH}/lib
    NO_DEFAULT_PATH
)
find_library(QGIS_GUI_LIBRARY
    NAMES qgis_gui
    PATHS
    ${LOCAL_QGIS_PATH}/lib
    NO_DEFAULT_PATH
)
find_library(QGIS_ANALYSIS_LIBRARY
    NAMES qgis_analysis
    PATHS
    ${LOCAL_QGIS_PATH}/lib
    NO_DEFAULT_PATH
)

IF(QGIS_INCLUDE_DIR)
    SET(QGIS_VERSION QGIS_VERSION-NOTFOUND)
    FIND_FILE(_qgsconfig_h qgsconfig.h PATHS ${QGIS_INCLUDE_DIR} NO_DEFAULT_PATH)

    IF(_qgsconfig_h)
        FILE(READ ${_qgsconfig_h} _qgsconfig)

        IF(_qgsconfig)
            # version defined like #define VERSION "2.14.8-Essen"
            FILE(STRINGS "${_qgsconfig_h}" _qgsversion_str REGEX "^#define VERSION .*$")
            STRING(REGEX REPLACE "^#define VERSION +\"([0-9]+\\.[0-9]+\\.[0-9]+).*$" "\\1" _qgsversion "${_qgsversion_str}")

            IF(_qgsversion)
                SET(QGIS_VERSION ${_qgsversion})
            ELSE()
                MESSAGE(WARNING "No QGIS version determined: failed to parse qgsconfig.h")
            ENDIF()
        ELSE()
            MESSAGE(WARNING "No QGIS version determined: failed to read qgsconfig.h")
        ENDIF()
    ELSE()
        MESSAGE(WARNING "No QGIS version determined: failed to find qgsconfig.h")
    ENDIF()
ENDIF()

IF(QGIS_INCLUDE_DIR AND QGIS_CORE_LIBRARY AND QGIS_GUI_LIBRARY AND QGIS_ANALYSIS_LIBRARY)
    SET(QGIS_FOUND TRUE)
ENDIF()

IF(NOT QGIS_FIND_QUIETLY)
    MESSAGE(STATUS "Found QGIS: ${QGIS_VERSION}")
    MESSAGE(STATUS "Found QGIS include dir: ${QGIS_INCLUDE_DIR}")
    MESSAGE(STATUS "Found QGIS core: ${QGIS_CORE_LIBRARY}")
    MESSAGE(STATUS "Found QGIS gui: ${QGIS_GUI_LIBRARY}")
    MESSAGE(STATUS "Found QGIS analysis: ${QGIS_ANALYSIS_LIBRARY}")
    MESSAGE(STATUS "Found QGIS plugins directory: ${QGIS_PLUGIN_DIR}")
ENDIF(NOT QGIS_FIND_QUIETLY)

IF(NOT QGIS_FOUND)
    IF(QGIS_FIND_REQUIRED)
        MESSAGE(FATAL_ERROR "Could not find QGIS. Please check if QGIS is installed in ${LOCAL_QGIS_PATH} or set 'OSGEO4W_ROOT' envvar.")
    ENDIF(QGIS_FIND_REQUIRED)
ENDIF(NOT QGIS_FOUND)