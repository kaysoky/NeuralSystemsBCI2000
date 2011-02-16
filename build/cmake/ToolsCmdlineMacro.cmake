###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Contains a macro for creating a commandline application

MACRO( BCI2000_ADD_TOOLS_CMDLINE NAME SOURCES HEADERS )

# DEBUG
MESSAGE( "-- Adding Commandline Project: " ${NAME} )

# Generate the required framework
INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/MinimalFramework.cmake )

# Add in the appropriate error handling module
SET( SRC_SHARED_BCISTREAM 
  ${BCI2000_SRC_DIR}/shared/bcistream/BCIError_tool.cpp
)
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\bcistream FILES ${SRC_SHARED_BCISTREAM} )
SET( SRC_BCI2000_FRAMEWORK
  ${SRC_BCI2000_FRAMEWORK}
  ${SRC_SHARED_BCISTREAM}
)

# Set the Project Source Groups
SOURCE_GROUP( Source\\Project FILES ${SOURCES} )
SOURCE_GROUP( Headers\\Project FILES ${HEADERS} )

# Add in external required libraries
BCI2000_SETUP_EXTLIB_DEPENDENCIES( SRC_BCI2000_FRAMEWORK HDR_BCI2000_FRAMEWORK LIBS )

# If we're building a Qt project, we need to automoc the sources, generating new files
IF( NOT BORLAND )
QT4_AUTOMOC( ${SOURCES} )

# Include Qt Modules specified elsewhere
INCLUDE ( ${QT_USE_FILE} )
ENDIF( NOT BORLAND )

# Add Pre-processor defines
ADD_DEFINITIONS( 
  -DNO_PCHINCLUDES
)

# Add the executable to the project
ADD_EXECUTABLE( ${NAME} ${SRC_BCI2000_FRAMEWORK} ${HDR_BCI2000_FRAMEWORK} ${SOURCES} ${HEADERS} )

# Set the output directories
SET_TARGET_PROPERTIES( ${NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${BCI2000_ROOT_DIR}/tools/cmdline )
SET_TARGET_PROPERTIES( ${NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${BCI2000_ROOT_DIR}/tools/cmdline )
SET_TARGET_PROPERTIES( ${NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${BCI2000_ROOT_DIR}/tools/cmdline )
IF( MSVC OR XCODE )
  SET_TARGET_PROPERTIES( ${NAME} PROPERTIES 
    PREFIX "../"
    IMPORT_PREFIX "../" 
  )
ENDIF( MSVC OR XCODE )

# Link against the Qt/VCL Libraries
IF( BORLAND )
TARGET_LINK_LIBRARIES( ${NAME} vcl rtl ${VXL_VGUI_LIBRARIES} ${LIBS} )
ELSE( BORLAND )
TARGET_LINK_LIBRARIES( ${NAME} ${QT_LIBRARIES} ${LIBS} )
ENDIF( BORLAND )

ENDMACRO( BCI2000_ADD_TOOLS_CMDLINE NAME SOURCES HEADERS )