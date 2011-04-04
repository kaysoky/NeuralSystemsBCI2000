###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Contains a macro for creating a GUI based tool application

MACRO( BCI2000_ADD_TOOLS_GUIAPP NAME SOURCES HEADERS GEN )

# DEBUG
MESSAGE( "-- Adding Tool Project: " ${NAME} )

# Generate the required framework
INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/MinimalFramework.cmake )

# Add Extra Sources
SET( SRC_BCI2000_FRAMEWORK
  ${SRC_BCI2000_FRAMEWORK}
  ${BCI2000_SRC_DIR}/shared/bcistream/BCIError_guiapp.cpp
  ${BCI2000_SRC_DIR}/shared/fileio/dat/BCI2000FileReader.cpp
  ${BCI2000_SRC_DIR}/shared/gui/AboutBox.cpp
  ${BCI2000_SRC_DIR}/shared/gui/ExecutableHelp.cpp
  ${BCI2000_SRC_DIR}/shared/utils/OSMutex.cpp
  ${BCI2000_SRC_DIR}/shared/utils/OSThread.cpp
  ${BCI2000_SRC_DIR}/shared/utils/Settings.cpp
)
SET( HDR_BCI2000_FRAMEWORK
  ${HDR_BCI2000_FRAMEWORK}
  ${BCI2000_SRC_DIR}/shared/fileio/dat/BCI2000FileReader.h
  ${BCI2000_SRC_DIR}/shared/gui/AboutBox.h
  ${BCI2000_SRC_DIR}/shared/gui/ExecutableHelp.h
  ${BCI2000_SRC_DIR}/shared/utils/OSMutex.h
  ${BCI2000_SRC_DIR}/shared/utils/OSThread.h
  ${BCI2000_SRC_DIR}/shared/utils/Settings.h
)
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\bcistream FILES
  ${BCI2000_SRC_DIR}/shared/bcistream/BCIError_guiapp.cpp )
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\fileio\\dat FILES
  ${BCI2000_SRC_DIR}/shared/fileio/dat/BCI2000FileReader.cpp )
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\gui FILES
  ${BCI2000_SRC_DIR}/shared/gui/AboutBox.cpp 
  ${BCI2000_SRC_DIR}/shared/gui/ExecutableHelp.cpp
)
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\utils FILES
  ${BCI2000_SRC_DIR}/shared/utils/OSMutex.cpp
  ${BCI2000_SRC_DIR}/shared/utils/OSThread.cpp
  ${BCI2000_SRC_DIR}/shared/utils/Settings.cpp
)
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\fileio\\dat FILES
  ${BCI2000_SRC_DIR}/shared/fileio/dat/BCI2000FileReader.h )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\gui FILES
  ${BCI2000_SRC_DIR}/shared/gui/AboutBox.h
  ${BCI2000_SRC_DIR}/shared/gui/ExecutableHelp.h
)
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\utils FILES
  ${BCI2000_SRC_DIR}/shared/utils/OSMutex.h
  ${BCI2000_SRC_DIR}/shared/utils/OSThread.h
  ${BCI2000_SRC_DIR}/shared/utils/Settings.h
)
  
IF( NOT BORLAND )
  SET( SRC_BCI2000_FRAMEWORK
    ${SRC_BCI2000_FRAMEWORK}
    ${BCI2000_SRC_DIR}/shared/gui/ColorListChooser.cpp
  )
  SET( HDR_BCI2000_FRAMEWORK
    ${HDR_BCI2000_FRAMEWORK}
    ${BCI2000_SRC_DIR}/shared/gui/ColorListChooser.h
  )
  SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\gui FILES
    ${BCI2000_SRC_DIR}/shared/gui/ColorListChooser.cpp
  )
  SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\gui FILES
    ${BCI2000_SRC_DIR}/shared/gui/ColorListChooser.h
  )
ENDIF( NOT BORLAND )

INCLUDE_DIRECTORIES(
  ${BCI2000_SRC_DIR}/shared/gui
  ${BCI2000_SRC_DIR}/shared/bcistream
  ${BCI2000_SRC_DIR}/shared/fileio/dat
)


# Set the Project Source Groups
SOURCE_GROUP( Source\\Project FILES ${SOURCES} )
SOURCE_GROUP( Headers\\Project FILES ${HEADERS} )

# Set Generated Source Groups
SOURCE_GROUP( Generated FILES ${GEN} )

# Add in external required libraries
BCI2000_SETUP_GUI_IMPORTS( SRC_BCI2000_FRAMEWORK HDR_BCI2000_FRAMEWORK )
BCI2000_SETUP_EXTLIB_DEPENDENCIES( SRC_BCI2000_FRAMEWORK HDR_BCI2000_FRAMEWORK LIBS )

# If we're building a Qt project, we need to automoc the sources, generating new files
IF( NOT BORLAND )
#QT4_AUTOMOC( ${SOURCES} )
#QT4_AUTOMOC( ${HEADERS} )

# Include Qt Modules specified elsewhere
#INCLUDE ( ${QT_USE_FILE} )
ENDIF( NOT BORLAND )

# Add Pre-processor defines
ADD_DEFINITIONS( 
  -DNO_PCHINCLUDES
)
IF( WIN32 )
ADD_DEFINITIONS(
  -D_WINDOWS
)
ENDIF( WIN32 )

# Add the executable to the project
ADD_EXECUTABLE( ${NAME} WIN32 ${SRC_BCI2000_FRAMEWORK} ${HDR_BCI2000_FRAMEWORK} ${SOURCES} ${HEADERS} ${GEN} )

# Set the output directories
SET_TARGET_PROPERTIES( ${NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${BCI2000_ROOT_DIR}/tools/${NAME} )
SET_TARGET_PROPERTIES( ${NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${BCI2000_ROOT_DIR}/tools/${NAME} )
SET_TARGET_PROPERTIES( ${NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${BCI2000_ROOT_DIR}/tools/${NAME} )
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

ENDMACRO( BCI2000_ADD_TOOLS_GUIAPP NAME SOURCES HEADERS GEN )
