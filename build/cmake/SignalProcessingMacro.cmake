###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Contains a macro for creating a signal processing module

MACRO( BCI2000_ADD_SIGNAL_PROCESSING_MODULE NAME SOURCES HEADERS SIGPROCSOURCES SIGPROCHEADERS INCLUDES )

# DEBUG
MESSAGE( "-- Adding Signal Processing Project: " ${NAME} )

INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/DefaultSigProcFramework.cmake )

# Add the required signal processing files into the framework for this project
SET( SRC_BCI2000_FRAMEWORK "${SRC_BCI2000_FRAMEWORK}" "${SIGPROCSOURCES}" "${EXTRASOURCES}" )
SET( HDR_BCI2000_FRAMEWORK "${HDR_BCI2000_FRAMEWORK}" "${SIGPROCHEADERS}" "${EXTRAHEADERS}" )

SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\modules\\signalprocessing FILES ${SIGPROCSOURCES} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\modules\\signalprocessing FILES ${SIGPROCHEADERS} )

# Set the Project Source Groups
SOURCE_GROUP( Source\\Project FILES ${SOURCES} )
SOURCE_GROUP( Headers\\Project FILES ${HEADERS} )

BCI2000_SETUP_EXTLIB_DEPENDENCIES( SRC_BCI2000_FRAMEWORK HDR_BCI2000_FRAMEWORK LIBS )

# If we're building a Qt project, we need to automoc the sources
IF( NOT BORLAND )
  SET(qtproject_SRCS
    ${SRC_BCI2000_FRAMEWORK}
    ${SOURCES}
  )
  QT4_AUTOMOC(${qtproject_SRCS})
ENDIF( NOT BORLAND )

# Add to our include directories
INCLUDE_DIRECTORIES( ${BCI2000_SRC_DIR}/shared/modules/signalprocessing ${INCLUDES} )

SET_OUTPUT_DIRECTORY( "${BCI2000_ROOT_DIR}/prog" )

# Add the executable to the project
ADD_EXECUTABLE( ${NAME} WIN32 ${SRC_BCI2000_FRAMEWORK} ${HDR_BCI2000_FRAMEWORK} ${SOURCES} ${HEADERS} )

# Add Pre-processor defines
SET_PROPERTY( TARGET ${NAME} APPEND PROPERTY COMPILE_FLAGS "-DMODTYPE=2" )
IF( NOT BORLAND )
  SET_PROPERTY( TARGET ${NAME} APPEND PROPERTY COMPILE_FLAGS "-DUSE_QT" )
ENDIF( NOT BORLAND )

# Link against the Qt/VCL Libraries
IF( BORLAND )
TARGET_LINK_LIBRARIES( ${NAME} vcl rtl ${VXL_VGUI_LIBRARIES} ${LIBS} )
ELSE( BORLAND )
TARGET_LINK_LIBRARIES( ${NAME} ${QT_LIBRARIES} ${LIBS} )
ENDIF( BORLAND )

ENDMACRO( BCI2000_ADD_SIGNAL_PROCESSING_MODULE NAME SOURCES HEADERS SIGPROCSOURCES SIGPROCHEADERS INCLUDES )