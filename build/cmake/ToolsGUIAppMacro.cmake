###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Contains a macro for creating a GUI based tool application

MACRO( BCI2000_ADD_TOOLS_GUIAPP NAME SOURCES HEADERS GEN )

# DEBUG
MESSAGE( "-- Adding Tool Project: " ${NAME} )

# Generate the required framework
INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/Core.cmake )

# Add Extra Sources
SET( SRC_BCI2000_FRAMEWORK
  ${SRC_BCI2000_FRAMEWORK}
  ${BCI2000_SRC_DIR}/shared/bcistream/BCIError_guiapp.cpp
  ${BCI2000_SRC_DIR}/shared/gui/AboutBox.cpp
  ${BCI2000_SRC_DIR}/shared/gui/ExecutableHelp.cpp
  ${BCI2000_SRC_DIR}/shared/utils/Settings.cpp
)
SET( HDR_BCI2000_FRAMEWORK
  ${HDR_BCI2000_FRAMEWORK}
  ${BCI2000_SRC_DIR}/shared/gui/AboutBox.h
  ${BCI2000_SRC_DIR}/shared/gui/ExecutableHelp.h
  ${BCI2000_SRC_DIR}/shared/utils/Settings.h
)
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\bcistream FILES
  ${BCI2000_SRC_DIR}/shared/bcistream/BCIError_guiapp.cpp )
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\gui FILES
  ${BCI2000_SRC_DIR}/shared/gui/AboutBox.cpp 
  ${BCI2000_SRC_DIR}/shared/gui/ExecutableHelp.cpp
)
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\utils FILES
  ${BCI2000_SRC_DIR}/shared/utils/Settings.cpp
)
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\gui FILES
  ${BCI2000_SRC_DIR}/shared/gui/AboutBox.h
  ${BCI2000_SRC_DIR}/shared/gui/ExecutableHelp.h
)
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\utils FILES
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
)


# Set the Project Source Groups
SOURCE_GROUP( Source\\Project FILES ${SOURCES} )
SOURCE_GROUP( Headers\\Project FILES ${HEADERS} )

# Set Generated Source Groups
SOURCE_GROUP( Generated FILES ${GEN} )

# Add in external required libraries
BCI2000_SETUP_GUI_IMPORTS( SRC_BCI2000_FRAMEWORK HDR_BCI2000_FRAMEWORK )
BCI2000_SETUP_EXTLIB_DEPENDENCIES( SRC_BCI2000_FRAMEWORK HDR_BCI2000_FRAMEWORK LIBS FAILED )

# If we're building a Qt project, we need to automoc the sources, generating new files
IF( NOT BORLAND )
  QT4_AUTOMOC( ${SOURCES} )
ENDIF( NOT BORLAND )

# Set the output directory
SET_OUTPUT_DIRECTORY( ${BCI2000_ROOT_DIR}/tools/${NAME} )

IF( NOT FAILED )
  BCI2000_ADD_TO_INVENTORY( Tool ${NAME} )
  
  # Add the executable to the project
  ADD_EXECUTABLE( ${NAME} WIN32 ${SRC_BCI2000_FRAMEWORK} ${HDR_BCI2000_FRAMEWORK} ${SOURCES} ${HEADERS} ${GEN} )

  # Add Pre-processor defines
  IF( NOT BORLAND )
    SET_PROPERTY( TARGET ${NAME} APPEND PROPERTY COMPILE_FLAGS "-DUSE_QT" )
  ENDIF( NOT BORLAND )

  # Link against the Qt/VCL Libraries
  IF( BORLAND )
    TARGET_LINK_LIBRARIES( ${NAME} vcl rtl ${VXL_VGUI_LIBRARIES} ${LIBS} )
  ELSE( BORLAND )
    TARGET_LINK_LIBRARIES( ${NAME} ${QT_LIBRARIES} ${LIBS} )
  ENDIF( BORLAND )

  # Set the project build folder
  SET_PROPERTY( TARGET ${NAME} PROPERTY FOLDER "${DIR_NAME}" )
ENDIF( NOT FAILED )

ENDMACRO( BCI2000_ADD_TOOLS_GUIAPP NAME SOURCES HEADERS GEN )
