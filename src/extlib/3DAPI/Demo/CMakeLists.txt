###########################################################################
## $Id: CMakeLists.txt 3497 2011-08-30 16:50:58Z mellinger $
## Authors: griffin.milsap@gmail.com
## Description: Build information for 3D API Demo in Qt

IF( BUILD_DEMOS )

SET( EXECUTABLE_NAME Demo3DAPI )
SET( DIR_NAME Demos/3DAPI )
BCI2000_INCLUDE( 3DAPI )
BCI2000_ADD_TOOLS_GUIAPP( 
  "${EXECUTABLE_NAME}" 
  Demo.cpp
  DemoWindow.cpp
  ${BCI2000_SRC_DIR}/shared/gui/GUI.cpp
  ${BCI2000_SRC_DIR}/shared/gui/GraphDisplay.cpp
  ${BCI2000_SRC_DIR}/shared/gui/GraphObject.cpp
)
SET_OUTPUT_DIRECTORY( ${CMAKE_CURRENT_SOURCE_DIR} ${EXECUTABLE_NAME} )

ENDIF( BUILD_DEMOS )
