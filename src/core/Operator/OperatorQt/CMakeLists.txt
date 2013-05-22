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
  MainWindow.ui
  ConnectionInfo.ui
  ShowStates.ui

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

  ${BCI2000_SRC_DIR}/core/Operator/OperatorLib/BCI_OperatorLib.h

  ${BCI2000_SRC_DIR}/shared/utils/DisplayFiltering/DisplayFilter.cpp
  ${BCI2000_SRC_DIR}/shared/utils/DisplayFiltering/DecimationFilter.cpp
  ${BCI2000_SRC_DIR}/shared/utils/DisplayFiltering/ScaleObservationFilter.cpp
  ${BCI2000_SRC_DIR}/shared/bcistream/BCIStream_guiapp.cpp
  ${BCI2000_SRC_DIR}/shared/gui/SignalDisplay.cpp
  ${BCI2000_SRC_DIR}/shared/gui/AboutBox.cpp
  ${BCI2000_SRC_DIR}/shared/gui/ColorListChooser.cpp
  ${BCI2000_SRC_DIR}/shared/utils/Settings.cpp 
)

BCI2000_INCLUDE( "MATH" )
BCI2000_IMPORT_GUI( "ConfigWindowQt" )
BCI2000_IMPORT_GUI( "EditMatrixQt" )
BCI2000_IMPORT_GUI( "ShowParametersQt" )
BCI2000_IMPORT_GUI( "PrefDialogQt" )
BCI2000_IMPORT_GUI( "PreferencesQt" )
BCI2000_IMPORT_GUI( "OperatorUtilsQt" )

INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/Core.cmake )

BCI2000_ADD_TO_INVENTORY( Operator ${EXECUTABLE_NAME} )
BCI2000_ADD_TARGET( QTAPP ${EXECUTABLE_NAME}  ${SRC_PROJECT} )
TARGET_LINK_LIBRARIES( ${EXECUTABLE_NAME} OperatorLib )

