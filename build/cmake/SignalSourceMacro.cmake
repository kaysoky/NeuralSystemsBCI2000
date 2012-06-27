###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Contains a macro for creating a signal source module

MACRO( BCI2000_ADD_SIGNAL_SOURCE_MODULE NAME SOURCES HEADERS )

# DEBUG
MESSAGE( "-- Adding Signal Source Project: " ${NAME} )

INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/SigSrcModule.cmake )

# Set the Project Source Groups
SOURCE_GROUP( Source\\Project FILES ${SOURCES} )
SOURCE_GROUP( Headers\\Project FILES ${HEADERS} )

# Setup Extlib Dependencies
BCI2000_SETUP_EXTLIB_DEPENDENCIES( SRC_BCI2000_FRAMEWORK HDR_BCI2000_FRAMEWORK LIBS FAILED )
BCI2000_ADD_CORE_MAIN( ${NAME} ${SOURCES} )

# If we're building a Qt project, we need to automoc the sources
IF( NOT BORLAND )
  QT4_AUTOMOC(${HEADERS})
ENDIF( NOT BORLAND )

IF( NOT FAILED )
  BCI2000_ADD_TO_INVENTORY( SignalSource ${NAME} )
  
  # Add the executable to the project
  ADD_EXECUTABLE( ${NAME} WIN32 ${SRC_BCI2000_FRAMEWORK} ${HDR_BCI2000_FRAMEWORK} ${SOURCES} ${HEADERS} )

  # Link against required libraries
  TARGET_LINK_LIBRARIES( ${NAME} ${LIBS} )

  # Add Pre-processor defines
  IF( NOT BORLAND )
    SET_PROPERTY( TARGET ${NAME} APPEND PROPERTY COMPILE_FLAGS "-DMODTYPE=1 -DUSE_QT" )
  ELSE( NOT BORLAND )
    SET_PROPERTY( TARGET ${NAME} APPEND PROPERTY COMPILE_FLAGS "-DMODTYPE=1" )
  ENDIF( NOT BORLAND )

  # Set the project build folder
  SET_PROPERTY( TARGET ${NAME} PROPERTY FOLDER "${DIR_NAME}" )
ENDIF( NOT FAILED )
ENDMACRO( BCI2000_ADD_SIGNAL_SOURCE_MODULE NAME SOURCES HEADERS )
