###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for OperatorQt

# Set the executable name
SET( EXECUTABLE_NAME Operator )

# DEBUG
MESSAGE( "-- Adding Project: ${EXECUTABLE_NAME}" )

# Set the project specific sources
SET( SRC_PROJECT
  main.cpp 
  MainWindow.cpp
  ConnectionInfo.cpp
  SysLog.cpp
  ShowStates.cpp
  VisDisplay.cpp
  VisDisplayBase.cpp
  VisDisplayMemo.cpp
  VisDisplayGraph.cpp
  VisDisplayBitmap.cpp
)
SET( HDR_PROJECT
  MainWindow.h
  ConnectionInfo.h
  SysLog.h
  ShowStates.h
  VisDisplay.h
  VisDisplayBase.h
  VisDisplayMemo.h
  VisDisplayGraph.h
  VisDisplayBitmap.h
)

# Wrap the Qt files
QT4_WRAP_UI( GENERATED_UI
  MainWindow.ui
  ConnectionInfo.ui
  ShowStates.ui
)
QT4_WRAP_CPP( GENERATED_MOC 
  MainWindow.h
  ConnectionInfo.h
  SysLog.h
  ShowStates.h
  VisDisplayBase.h
  VisDisplayMemo.h
  VisDisplayGraph.h
  VisDisplayBitmap.h
)
SET( GENERATED
  ${GENERATED_UI}
  ${GENERATED_MOC}
)

# Tell the macro to link against the math libraries in extlib
BCI2000_USE( "MATH" )

# Tell the macro to link against a few external GUIs
BCI2000_IMPORT_GUI( "ConfigWindowQt" )
BCI2000_IMPORT_GUI( "EditMatrixQt" )
BCI2000_IMPORT_GUI( "ShowParametersQt" )
BCI2000_IMPORT_GUI( "PrefDialogQt" )
BCI2000_IMPORT_GUI( "PreferencesQt" )
BCI2000_IMPORT_GUI( "OperatorUtilsQt" )

# Generate the required framework
INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/MinimalFramework.cmake )

# Add missing framework files
SET( SRC_BCI2000_FRAMEWORK
  ${SRC_BCI2000_FRAMEWORK}
  ${BCI2000_SRC_DIR}/shared/fileio/dat/BCI2000FileReader.cpp
  ${BCI2000_SRC_DIR}/shared/bcistream/BCIError_guiapp.cpp
  ${BCI2000_SRC_DIR}/shared/utils/DisplayFilter.cpp
  ${BCI2000_SRC_DIR}/shared/utils/DecimationFilter.cpp
  ${BCI2000_SRC_DIR}/shared/utils/OSThread.cpp
  ${BCI2000_SRC_DIR}/shared/utils/OSMutex.cpp
  ${BCI2000_SRC_DIR}/shared/utils/Settings.cpp 
  ${BCI2000_SRC_DIR}/shared/gui/SignalDisplay.cpp
  ${BCI2000_SRC_DIR}/shared/gui/AboutBox.cpp
  ${BCI2000_SRC_DIR}/shared/gui/ColorListChooser.cpp
)
SET( HDR_BCI2000_FRAMEWORK
  ${HDR_BCI2000_FRAMEWORK}
  ${BCI2000_SRC_DIR}/shared/fileio/dat/BCI2000FileReader.h
  ${BCI2000_SRC_DIR}/shared/utils/DisplayFilter.h
  ${BCI2000_SRC_DIR}/shared/utils/DecimationFilter.h
  ${BCI2000_SRC_DIR}/shared/utils/OSThread.h
  ${BCI2000_SRC_DIR}/shared/utils/OSMutex.h
  ${BCI2000_SRC_DIR}/shared/utils/Settings.h 
  ${BCI2000_SRC_DIR}/shared/gui/SignalDisplay.h
  ${BCI2000_SRC_DIR}/shared/gui/AboutBox.h
  ${BCI2000_SRC_DIR}/shared/gui/ColorListChooser.h
)

SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\fileio\\dat FILES
  ${BCI2000_SRC_DIR}/shared/fileio/dat/BCI2000FileReader.cpp 
)
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\bcistream FILES
  ${BCI2000_SRC_DIR}/shared/bcistream/BCIError_guiapp.cpp 
)
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\utils FILES
  ${BCI2000_SRC_DIR}/shared/utils/DisplayFilter.cpp
  ${BCI2000_SRC_DIR}/shared/utils/DecimationFilter.cpp
  ${BCI2000_SRC_DIR}/shared/utils/OSThread.cpp
  ${BCI2000_SRC_DIR}/shared/utils/OSMutex.cpp 
  ${BCI2000_SRC_DIR}/shared/utils/Settings.cpp 
)
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\gui FILES
  ${BCI2000_SRC_DIR}/shared/gui/SignalDisplay.cpp
  ${BCI2000_SRC_DIR}/shared/gui/AboutBox.cpp
  ${BCI2000_SRC_DIR}/shared/gui/ColorListChooser.cpp
)
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\fileio\\dat FILES
  ${BCI2000_SRC_DIR}/shared/fileio/dat/BCI2000FileReader.h 
)
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\utils FILES
  ${BCI2000_SRC_DIR}/shared/utils/DisplayFilter.h
  ${BCI2000_SRC_DIR}/shared/utils/DecimationFilter.h
  ${BCI2000_SRC_DIR}/shared/utils/OSThread.h
  ${BCI2000_SRC_DIR}/shared/utils/OSMutex.h 
  ${BCI2000_SRC_DIR}/shared/utils/Settings.h 
)
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\gui FILES
  ${BCI2000_SRC_DIR}/shared/gui/SignalDisplay.h
  ${BCI2000_SRC_DIR}/shared/gui/AboutBox.h
  ${BCI2000_SRC_DIR}/shared/gui/ColorListChooser.h
)


SET( HDR_PROJECT
     ${HDR_PROJECT}
     ${BCI2000_SRC_DIR}/core/Operator/OperatorLib/BCI_OperatorLib.h
)
INCLUDE_DIRECTORIES( ${BCI2000_SRC_DIR}/core/Operator/OperatorLib )

# Set the Project Source Groups
SOURCE_GROUP( Source\\Project FILES ${SRC_PROJECT} )
SOURCE_GROUP( Headers\\Project FILES ${HDR_PROJECT} )

# Set Generated Source Groups
SOURCE_GROUP( Generated FILES ${GENERATED} )

# Setup Extlib Dependencies
BCI2000_SETUP_EXTLIB_DEPENDENCIES( SRC_BCI2000_FRAMEWORK HDR_BCI2000_FRAMEWORK LIBS )

# Setup GUI Imports
BCI2000_SETUP_GUI_IMPORTS( SRC_BCI2000_FRAMEWORK HDR_BCI2000_FRAMEWORK )

# If we're building a Qt project, we need to automoc the sources, generating new files
QT4_AUTOMOC( ${SRC_PROJECT} )
QT4_AUTOMOC( ${HDR_PROJECT} )

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
ADD_EXECUTABLE( 
  ${EXECUTABLE_NAME} 
  WIN32
  ${SRC_BCI2000_FRAMEWORK} ${HDR_BCI2000_FRAMEWORK} 
  ${SRC_PROJECT} ${HDR_PROJECT} 
  ${GENERATED} 
)

TARGET_LINK_LIBRARIES( ${EXECUTABLE_NAME} OperatorLib )

# Set the output directories
SET_TARGET_PROPERTIES( ${EXECUTABLE_NAME} PROPERTIES 
  RUNTIME_OUTPUT_DIRECTORY ${BCI2000_ROOT_DIR}/prog 
)
IF( MSVC OR XCODE )
  SET_TARGET_PROPERTIES( ${EXECUTABLE_NAME} PROPERTIES 
    PREFIX "../"
    IMPORT_PREFIX "../" 
  )
ENDIF( MSVC OR XCODE )

# Link against the Qt Libraries and other dependencies
TARGET_LINK_LIBRARIES( ${EXECUTABLE_NAME} ${QT_LIBRARIES} ${LIBS} )

