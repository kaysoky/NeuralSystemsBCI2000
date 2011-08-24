###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Contains a macro for creating an application module

MACRO( BCI2000_ADD_APPLICATION_MODULE NAME SOURCES HEADERS APPSOURCES APPHEADERS )

# DEBUG
MESSAGE( "-- Adding Application Project: " ${NAME} )

# Generate the required framework
INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/AppModuleFramework.cmake )

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
ADD_EXECUTABLE( ${NAME} WIN32 ${SRC_BCI2000_FRAMEWORK} ${HDR_BCI2000_FRAMEWORK} ${APPSOURCES} ${APPHEADERS} ${SOURCES} ${HEADERS} )

# Add Pre-processor defines
IF( NOT BORLAND )
  SET_PROPERTY( TARGET ${NAME} APPEND PROPERTY COMPILE_FLAGS "-DMODTYPE=3 -DUSE_QT" )
ELSE( NOT BORLAND )
  SET_PROPERTY( TARGET ${NAME} APPEND PROPERTY COMPILE_FLAGS "-DMODTYPE=3" )
ENDIF( NOT BORLAND )

# Link against the Qt/VCL Libraries
IF( BORLAND )
  TARGET_LINK_LIBRARIES( ${NAME} vcl rtl ${VXL_VGUI_LIBRARIES} ${LIBS} cp32mt.lib )
ELSE( BORLAND )
  TARGET_LINK_LIBRARIES( ${NAME} ${QT_LIBRARIES} ${LIBS} )
ENDIF( BORLAND )

ENDMACRO( BCI2000_ADD_APPLICATION_MODULE NAME SOURCES HEADERS APPSOURCES APPHEADERS )