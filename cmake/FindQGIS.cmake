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

IF(WIN32)
    # OSGEO4W_QGIS_SUBDIR relative install: qgis[-rel|-ltr][-dev], etc.
    IF(NOT OSGEO4W_QGIS_SUBDIR OR "${OSGEO4W_QGIS_SUBDIR}" STREQUAL "")
       IF(NOT "$ENV{OSGEO4W_QGIS_SUBDIR}" STREQUAL "")
           SET(OSGEO4W_QGIS_SUBDIR $ENV{OSGEO4W_QGIS_SUBDIR})
       ELSE()
           SET(OSGEO4W_QGIS_SUBDIR qgis)
       ENDIF()
    ENDIF()
    # set(OSGEO4W_QGIS_SUBDIR qgis)

    FIND_PATH(QGIS_PLUGIN_DIR
        NAMES plugin_offlineediting.dll
        PATHS
        "$ENV{OSGEO4W_ROOT}/apps/${OSGEO4W_QGIS_SUBDIR}/plugins"
        "$ENV{PROGRAMFILES}/QGIS/plugins"
        NO_CMAKE_FIND_ROOT_PATH
    )
    FIND_PATH(QGIS_INCLUDE_DIR
        NAMES qgsapplication.h
        PATHS
        "$ENV{INCLUDE}"
        "$ENV{LIB_DIR}/include/qgis"
        "$ENV{OSGEO4W_ROOT}/include"
        "$ENV{OSGEO4W_ROOT}/apps/${OSGEO4W_QGIS_SUBDIR}/include"
        "$ENV{OSGEO4W_ROOT}/apps/${OSGEO4W_QGIS_SUBDIR}/include/qgis"
        "$ENV{PROGRAMFILES}/QGIS/include"
        NO_CMAKE_FIND_ROOT_PATH
    )
    FIND_LIBRARY(QGIS_CORE_LIBRARY
        NAMES qgis_core
        PATHS
        "$ENV{LIB_DIR}/lib/"
        "$ENV{LIB}"
        "$ENV{OSGEO4W_ROOT}/lib"
        "$ENV{OSGEO4W_ROOT}/apps/${OSGEO4W_QGIS_SUBDIR}/lib"
        "$ENV{PROGRAMFILES}/QGIS/lib"
        NO_CMAKE_FIND_ROOT_PATH
    )
    FIND_LIBRARY(QGIS_GUI_LIBRARY
        NAMES qgis_gui
        PATHS
        "$ENV{LIB_DIR}"
        "$ENV{LIB}"
        "$ENV{OSGEO4W_ROOT}/lib"
        "$ENV{OSGEO4W_ROOT}/apps/${OSGEO4W_QGIS_SUBDIR}/lib"
        "$ENV{PROGRAMFILES}/QGIS/lib"
        NO_CMAKE_FIND_ROOT_PATH
    )
    FIND_LIBRARY(QGIS_ANALYSIS_LIBRARY
        NAMES qgis_analysis
        PATHS
        "$ENV{LIB_DIR}"
        "$ENV{LIB}"
        "$ENV{OSGEO4W_ROOT}/lib"
        "$ENV{OSGEO4W_ROOT}/apps/${OSGEO4W_QGIS_SUBDIR}/lib"
        "$ENV{PROGRAMFILES}/QGIS/lib"
        NO_CMAKE_FIND_ROOT_PATH
    )
ELSE(WIN32)
    IF(UNIX)
        # MESSAGE("Searching for QGIS in /usr/bin; /usr/local/bin")
        FIND_PATH(QGIS_PLUGIN_DIR
            NAMES libplugin_offlineediting.so
            PATHS
            ${QGIS_BUILD_PATH}/PlugIns/qgis
            ${QGIS_MAC_PATH}/PlugIns/qgis
            ${QGIS_PREFIX_PATH}/lib/qgis/plugins
            /usr/lib64/qgis/plugins
            /usr/lib/qgis
            /usr/lib/qgis/plugins
            /usr/local/lib/qgis/plugins
            "$ENV{LIB_DIR}/lib/qgis/plugins"
            "$ENV{LIB_DIR}/lib/qgis"
        )
        FIND_PATH(QGIS_INCLUDE_DIR
            NAMES qgis.h
            PATHS
            ${QGIS_BUILD_PATH}/output/lib/qgis_core.framework/Headers
            ${QGIS_MAC_PATH}/Frameworks/qgis_core.framework/Headers
            ${QGIS_PREFIX_PATH}/include/qgis
            /usr/include/qgis
            /usr/local/include/qgis
            /Library/Frameworks/qgis_core.framework/Headers
            "$ENV{LIB_DIR}/include/qgis"
        )
        FIND_PATH(QGIS_UI_INCLUDE_DIR
            NAMES ui_qgscredentialdialog.h
            PATHS
            ${QGIS_BUILD_PATH}/src/ui
            ${QGIS_MAC_PATH}/Frameworks/qgis_gui.framework/Headers
            ${QGIS_PREFIX_PATH}/include/qgis
            /usr/include/qgis
            /usr/local/include/qgis
            /Library/Frameworks/qgis_gui.framework/Headers
            "$ENV{LIB_DIR}/include/qgis"
        )

        # also get other frameworks' headers folders on OS X
        IF(APPLE)
            FIND_PATH(QGIS_GUI_INCLUDE_DIR
                NAMES qgsguiutils.h
                PATHS
                ${QGIS_BUILD_PATH}/output/lib
                ${QGIS_MAC_PATH}/Frameworks
                /Library/Frameworks
                PATH_SUFFIXES qgis_gui.framework/Headers
            )
            FIND_PATH(QGIS_ANALYSIS_INCLUDE_DIR
                NAMES qgsinterpolator.h
                PATHS
                ${QGIS_BUILD_PATH}/output/lib
                ${QGIS_MAC_PATH}/Frameworks
                /Library/Frameworks
                PATH_SUFFIXES qgis_analysis.framework/Headers
            )
            SET(QGIS_INCLUDE_DIR
                ${QGIS_INCLUDE_DIR}
                ${QGIS_GUI_INCLUDE_DIR}
                ${QGIS_ANALYSIS_INCLUDE_DIR}
                ${QGIS_UI_INCLUDE_DIR}
            )
        ENDIF(APPLE)

        FIND_LIBRARY(QGIS_CORE_LIBRARY
            NAMES qgis_core
            PATHS
            ${QGIS_BUILD_PATH}/output/lib
            ${QGIS_PREFIX_PATH}/lib
            ${QGIS_MAC_PATH}/Frameworks
            ${QGIS_MAC_PATH}/lib
            /usr/lib64
            /usr/lib
            /usr/local/lib
            /Library/Frameworks
            "$ENV{LIB_DIR}/lib/"
            NO_DEFAULT_PATH
        )
        FIND_LIBRARY(QGIS_GUI_LIBRARY
            NAMES qgis_gui
            PATHS
            ${QGIS_BUILD_PATH}/output/lib
            ${QGIS_PREFIX_PATH}/lib
            ${QGIS_MAC_PATH}/Frameworks
            ${QGIS_MAC_PATH}/lib
            /usr/lib64
            /usr/lib
            /usr/local/lib
            /Library/Frameworks
            "$ENV{LIB_DIR}/lib/"
            NO_DEFAULT_PATH
        )
        FIND_LIBRARY(QGIS_ANALYSIS_LIBRARY
            NAMES qgis_analysis
            PATHS
            ${QGIS_BUILD_PATH}/output/lib
            ${QGIS_PREFIX_PATH}/lib
            ${QGIS_MAC_PATH}/Frameworks
            ${QGIS_MAC_PATH}/lib
            /usr/lib64
            /usr/lib
            /usr/local/lib
            /Library/Frameworks
            "$ENV{LIB_DIR}/lib/"
            NO_DEFAULT_PATH
        )
    ENDIF(UNIX)
ENDIF(WIN32)

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
