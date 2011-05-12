###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Contains a macro for creating an application module

MACRO( BCI2000_ADD_APPLICATION_MODULE NAME SOURCES HEADERS APPSOURCES APPHEADERS INCLUDES )

# DEBUG
MESSAGE( "-- Adding Application Project: " ${NAME} )

# Generate the required framework
INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/DefaultAppFramework.cmake )

# Setup Global Application Modules
INCLUDE( ${BCI2000_CMAKE_DIR}/ApplicationModules.cmake )

# Add the global and specific application files into the framework for this project
SET( SRC_BCI2000_FRAMEWORK "${SRC_BCI2000_FRAMEWORK}" "${APPSOURCES}" "${BCI2000_APPSOURCES}" )
SET( HDR_BCI2000_FRAMEWORK "${HDR_BCI2000_FRAMEWORK}" "${APPHEADERS}" "${BCI2000_APPHEADERS}" )

# Set the Project Source Groups
SOURCE_GROUP( Source\\Project FILES ${SOURCES} )
SOURCE_GROUP( Headers\\Project FILES ${HEADERS} )

# Add in external required libraries
BCI2000_SETUP_EXTLIB_DEPENDENCIES( SRC_BCI2000_FRAMEWORK HDR_BCI2000_FRAMEWORK LIBS )

# If we're building a Qt project, we need to automoc the sources, generating new files
IF( NOT BORLAND )
  QT4_AUTOMOC( ${SOURCES} )

  # Moc Framework Sources differently, as Automoc doesn't like relative paths.
  QT_WRAP_CPP( ${NAME} GENERATED ${HDR_BCI2000_MOC} )
  SOURCE_GROUP( Generated FILES ${GENERATED} )
  SET( SRC_BCI2000_FRAMEWORK
    ${SRC_BCI2000_FRAMEWORK}
    ${GENERATED}
  )
ENDIF( NOT BORLAND )

# Set Application Include Directories
INCLUDE_DIRECTORIES( ${INCLUDES} )

# Add the executable to the project
ADD_EXECUTABLE( ${NAME} WIN32 ${SRC_BCI2000_FRAMEWORK} ${HDR_BCI2000_FRAMEWORK} ${SOURCES} ${HEADERS} )

# Add Pre-processor defines
SET_PROPERTY( TARGET ${NAME} APPEND PROPERTY COMPILE_FLAGS "-DMODTYPE=3 -DUSE_QT" )

# Link against the Qt/VCL Libraries
IF( BORLAND )
TARGET_LINK_LIBRARIES( ${NAME} vcl rtl ${VXL_VGUI_LIBRARIES} ${LIBS} cp32mt.lib )
ELSE( BORLAND )
TARGET_LINK_LIBRARIES( ${NAME} ${QT_LIBRARIES} ${LIBS} )
ENDIF( BORLAND )

ENDMACRO( BCI2000_ADD_APPLICATION_MODULE NAME SOURCES HEADERS APPSOURCES APPHEADERS INCLUDES )