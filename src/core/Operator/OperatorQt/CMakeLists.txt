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
  ../OperatorLib/ScriptParser/ParserToken.cpp
  ConnectionInfo.cpp
  SysLog.cpp
  ShowStates.cpp
  VisDisplay.cpp
  VisDisplayWindow.cpp
  VisDisplayLayer.cpp
  VisDisplayBase.cpp
  VisDisplayMemo.cpp
  VisDisplayGraph.cpp
  VisDisplayBitmap.cpp
)
SET( HDR_PROJECT
  MainWindow.h
  ../OperatorLib/ScriptParser/ParserToken.cpp
  ConnectionInfo.h
  SysLog.h
  ShowStates.h
  VisDisplay.h
  VisDisplayWindow.h
  VisDisplayLayer.h
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
  VisDisplayWindow.h
  VisDisplayLayer.h
  VisDisplayMemo.h
  VisDisplayGraph.h
  VisDisplayBitmap.h
)
SET( GENERATED
  ${GENERATED_UI}
  ${GENERATED_MOC}
)

# Tell the macro to link against the math libraries in extlib
BCI2000_INCLUDE( "MATH" )

# Tell the macro to link against a few external GUIs
BCI2000_IMPORT_GUI( "ConfigWindowQt" )
BCI2000_IMPORT_GUI( "EditMatrixQt" )
BCI2000_IMPORT_GUI( "ShowParametersQt" )
BCI2000_IMPORT_GUI( "PrefDialogQt" )
BCI2000_IMPORT_GUI( "PreferencesQt" )
BCI2000_IMPORT_GUI( "OperatorUtilsQt" )

# Generate the required framework
INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/Core.cmake )

# Add missing framework files
SET( SRC_BCI2000_DISPLAYFILTERING
  ${BCI2000_SRC_DIR}/shared/utils/DisplayFiltering/DisplayFilter.cpp
  ${BCI2000_SRC_DIR}/shared/utils/DisplayFiltering/DecimationFilter.cpp
  ${BCI2000_SRC_DIR}/shared/utils/DisplayFiltering/ScaleObservationFilter.cpp
)
SET( HDR_BCI2000_DISPLAYFILTERING
  ${BCI2000_SRC_DIR}/shared/utils/DisplayFiltering/DisplayFilter.h
  ${BCI2000_SRC_DIR}/shared/utils/DisplayFiltering/DecimationFilter.h
  ${BCI2000_SRC_DIR}/shared/utils/DisplayFiltering/ScaleObservationFilter.h
)
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\utils\\DisplayFiltering FILES
  ${SRC_BCI2000_DISPLAYFILTERING}
)
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\utils\\DisplayFiltering FILES
  ${HDR_BCI2000_DISPLAYFILTERING}
)

SET( SRC_BCI2000_FRAMEWORK
  ${SRC_BCI2000_FRAMEWORK}
  ${SRC_BCI2000_DISPLAYFILTERING}
  ${BCI2000_SRC_DIR}/shared/bcistream/BCIError_guiapp.cpp
  ${BCI2000_SRC_DIR}/shared/utils/Settings.cpp 
  ${BCI2000_SRC_DIR}/shared/gui/SignalDisplay.cpp
  ${BCI2000_SRC_DIR}/shared/gui/AboutBox.cpp
  ${BCI2000_SRC_DIR}/shared/gui/ColorListChooser.cpp
)
SET( HDR_BCI2000_FRAMEWORK
  ${HDR_BCI2000_FRAMEWORK}
  ${HDR_BCI2000_DISPLAYFILTERING}
  ${BCI2000_SRC_DIR}/shared/utils/Settings.h 
  ${BCI2000_SRC_DIR}/shared/gui/SignalDisplay.h
  ${BCI2000_SRC_DIR}/shared/gui/AboutBox.h
  ${BCI2000_SRC_DIR}/shared/gui/ColorListChooser.h
)

SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\bcistream FILES
  ${BCI2000_SRC_DIR}/shared/bcistream/BCIError_guiapp.cpp 
)
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\utils FILES
  ${BCI2000_SRC_DIR}/shared/utils/Settings.cpp 
)
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\gui FILES
  ${BCI2000_SRC_DIR}/shared/gui/SignalDisplay.cpp
  ${BCI2000_SRC_DIR}/shared/gui/AboutBox.cpp
  ${BCI2000_SRC_DIR}/shared/gui/ColorListChooser.cpp
)
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\utils FILES
  ${BCI2000_SRC_DIR}/shared/utils/DisplayFilter.h
  ${BCI2000_SRC_DIR}/shared/utils/DecimationFilter.h
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
INCLUDE_DIRECTORIES(
  ${BCI2000_SRC_DIR}/core/Operator/OperatorLib
  ${BCI2000_SRC_DIR}/core/Operator/OperatorLib/ScriptParser
  ${BCI2000_SRC_DIR}/shared/utils/DisplayFiltering
)

# Set the Project Source Groups
SOURCE_GROUP( Source\\Project FILES ${SRC_PROJECT} )
SOURCE_GROUP( Headers\\Project FILES ${HDR_PROJECT} )

# Set Generated Source Groups
SOURCE_GROUP( Generated FILES ${GENERATED} )

# Setup Extlib Dependencies
BCI2000_SETUP_EXTLIB_DEPENDENCIES( SRC_BCI2000_FRAMEWORK HDR_BCI2000_FRAMEWORK LIBS FAILED )

# Setup GUI Imports
BCI2000_SETUP_GUI_IMPORTS( SRC_BCI2000_FRAMEWORK HDR_BCI2000_FRAMEWORK )

IF( NOT FAILED )
  BCI2000_ADD_TO_INVENTORY( Operator ${EXECUTABLE_NAME} )
  
  # Add the executable to the project
  ADD_EXECUTABLE( 
    ${EXECUTABLE_NAME} 
    WIN32
    ${SRC_BCI2000_FRAMEWORK} ${HDR_BCI2000_FRAMEWORK} 
    ${SRC_PROJECT} ${HDR_PROJECT} 
    ${GENERATED} 
  )
  # Add Pre-processor defines
  SET_PROPERTY( TARGET ${EXECUTABLE_NAME} APPEND PROPERTY COMPILE_FLAGS "-DUSE_QT" )
  TARGET_LINK_LIBRARIES( ${EXECUTABLE_NAME} OperatorLib )
  # Link against the Qt Libraries and other dependencies
  TARGET_LINK_LIBRARIES( ${EXECUTABLE_NAME} ${QT_LIBRARIES} ${LIBS} )

  # Set the project build folder
  SET_PROPERTY( TARGET ${EXECUTABLE_NAME} PROPERTY FOLDER "${DIR_NAME}" )
ENDIF( NOT FAILED )

