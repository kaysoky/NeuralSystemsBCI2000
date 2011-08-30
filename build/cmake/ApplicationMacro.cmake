###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Contains a macro for creating an application module

MACRO( BCI2000_ADD_APPLICATION_MODULE NAME SOURCES HEADERS )

# DEBUG
MESSAGE( "-- Adding Application Project: " ${NAME} )

# Depend on the app module framework library
INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/AppModule.cmake )

# Set the Project Source Groups
SOURCE_GROUP( Source\\Project FILES ${SOURCES} )
SOURCE_GROUP( Headers\\Project FILES ${HEADERS} )

# Add in external required libraries
BCI2000_SETUP_EXTLIB_DEPENDENCIES( SRC_BCI2000_FRAMEWORK HDR_BCI2000_FRAMEWORK LIBS )

# If we're building a Qt project, we need to automoc the sources, generating new files
IF( NOT BORLAND )
  QT4_AUTOMOC( ${SOURCES} )
ENDIF( NOT BORLAND )

# Add the executable to the project
ADD_EXECUTABLE( ${NAME} WIN32 ${SRC_BCI2000_FRAMEWORK} ${HDR_BCI2000_FRAMEWORK} ${SOURCES} ${HEADERS} )

# Link against required libraries
TARGET_LINK_LIBRARIES( ${NAME} ${LIBS} )

# Add Pre-processor defines
IF( NOT BORLAND )
  SET_PROPERTY( TARGET ${NAME} APPEND PROPERTY COMPILE_FLAGS "-DMODTYPE=3 -DUSE_QT" )
ELSE( NOT BORLAND )
  SET_PROPERTY( TARGET ${NAME} APPEND PROPERTY COMPILE_FLAGS "-DMODTYPE=3" )
ENDIF( NOT BORLAND )

ENDMACRO( BCI2000_ADD_APPLICATION_MODULE NAME SOURCES HEADERS )
