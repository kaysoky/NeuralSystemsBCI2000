###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Contains a macro for creating a signal source module

INCLUDE( ${BCI2000_CMAKE_DIR}/SignalSourceModules.cmake )

MACRO( BCI2000_ADD_SIGNAL_SOURCE_MODULE NAME SOURCES HEADERS INCLUDES )

# DEBUG
MESSAGE( "-- Adding Signal Source Project: " ${NAME} )

INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/DefaultSigSrcFramework.cmake )

# Add the required signal processing files into the framework for this project
SET( SRC_BCI2000_FRAMEWORK "${SRC_BCI2000_FRAMEWORK}" "${BCI2000_SIGSRCSOURCES}" "${BCI2000_SIGSRCSOURCES_EXTENSIONS}" "${BCI2000_SIGSRCSOURCES_LOGGING}" )
SET( HDR_BCI2000_FRAMEWORK "${HDR_BCI2000_FRAMEWORK}" "${BCI2000_SIGSRCHEADERS}" "${BCI2000_SIGSRCHEADERS_EXTENSIONS}" "${BCI2000_SIGSRCHEADERS_LOGGING}" )

SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\modules\\signalsource FILES ${BCI2000_SIGSRCSOURCES} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\modules\\signalsource FILES ${BCI2000_SIGSRCHEADERS} )
SOURCE_GROUP( Source\\BCI2000_Framework\\contrib\\extensions FILES ${BCI2000_SIGSRCSOURCES_EXTENSIONS} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\contrib\\extensions FILES ${BCI2000_SIGSRCHEADERS_EXTENSIONS} )
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\modules\\signalsource\\logging FILES ${BCI2000_SIGSRCSOURCES_LOGGING} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\modules\\signalsource\\logging FILES ${BCI2000_SIGSRCHEADERS_LOGGING} )

# Set the Project Source Groups
SOURCE_GROUP( Source\\Project FILES ${SOURCES} )
SOURCE_GROUP( Headers\\Project FILES ${HEADERS} )

# Setup Extlib Dependencies
BCI2000_SETUP_EXTLIB_DEPENDENCIES( SRC_BCI2000_FRAMEWORK HDR_BCI2000_FRAMEWORK LIBS )

# If we're building a Qt project, we need to automoc the sources
IF( NOT BORLAND )
SET(qtproject_SRCS
  "${SRC_BCI2000_FRAMEWORK}"
  "${SOURCES}"
)
QT4_AUTOMOC(${qtproject_SRCS})
ENDIF( NOT BORLAND )

# Add to our include directories
INCLUDE_DIRECTORIES( ${BCI2000_SRC_DIR}/shared/modules/signalsource ${INCLUDES} ${BCI2000_SIGSRCINCDIRS} )

# Add link directories for signal source modules
LINK_DIRECTORIES( ${BCI2000_SIGSRCLIBDIRS} )

# Add the executable to the project
ADD_EXECUTABLE( ${NAME} WIN32 ${SRC_BCI2000_FRAMEWORK} ${HDR_BCI2000_FRAMEWORK} ${SOURCES} ${HEADERS} )

# Add Pre-processor defines
SET_PROPERTY( TARGET ${NAME} APPEND PROPERTY COMPILE_FLAGS "-DMODTYPE=1" )
IF( NOT BORLAND )
  SET_PROPERTY( TARGET ${NAME} APPEND PROPERTY COMPILE_FLAGS "-DUSE_QT" )
ENDIF( NOT BORLAND )

# Link against the Qt/VCL Libraries
IF( BORLAND )
TARGET_LINK_LIBRARIES( ${NAME} vcl rtl ${VXL_VGUI_LIBRARIES} ${LIBS} ${BCI2000_SIGSRCLIBS} )
ELSE( BORLAND )
TARGET_LINK_LIBRARIES( ${NAME} ${QT_LIBRARIES} ${LIBS} ${BCI2000_SIGSRCLIBS} )
ENDIF( BORLAND )

# Copy required DLLs
FOREACH( SIGSRCDLL ${BCI2000_SIGSRCDLLS} )
  ADD_CUSTOM_COMMAND(
    TARGET ${NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${SIGSRCDLL}" "${BCI2000_ROOT_DIR}/prog"
  )
ENDFOREACH( SIGSRCDLL )

ENDMACRO( BCI2000_ADD_SIGNAL_SOURCE_MODULE NAME SOURCES HEADERS INCLUDES )