###########################################################################
## $Id: CMakeLists.txt 3497 2011-08-30 16:50:58Z mellinger $
## Authors: griffin.milsap@gmail.com
## Description: Build information for 3D API Demo in Qt

# Set the executable name
SET( EXECUTABLE_NAME Demo3DAPI )

SET( SRC_FRAMEWORK
  ${BCI2000_SRC_DIR}/shared/gui/GUI.cpp
  ${BCI2000_SRC_DIR}/shared/gui/GraphDisplay.cpp
  ${BCI2000_SRC_DIR}/shared/gui/GraphObject.cpp
)
SET( HDR_FRAMEWÓRK
  ${BCI2000_SRC_DIR}/shared/gui/GUI.h
  ${BCI2000_SRC_DIR}/shared/gui/GraphDisplay.h
  ${BCI2000_SRC_DIR}/shared/gui/GraphObject.h
)
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\gui FILES ${SRC_FRAMEWORK} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\gui FILES ${HDR_FRAMEWORK} )

# Set the project specific sources
SET( SRC_PROJECT
  Demo.cpp
  DemoWindow.cpp
)
SET( HDR_PROJECT
  DemoWindow.h
)

QT4_WRAP_CPP( GENERATED
  ${HDR_PROJECT}
)

SET( SRC
 ${SRC_FRAMEWORK}
 ${SRC_PROJECT}
)
SET( HDR
 ${HDR_FRAMEWORK}
 ${HDR_PROJECT}
)

# Define extra include directories
INCLUDE_DIRECTORIES(
)

# Let the macro know we want to use the 3D API
BCI2000_INCLUDE( "3DAPI" )

# Create the module
BCI2000_ADD_TOOLS_GUIAPP( 
  "${EXECUTABLE_NAME}" 
  "${SRC}" "${HDR}" 
  "${GENERATED}"
)

SET_OUTPUT_DIRECTORY( ${CMAKE_CURRENT_SOURCE_DIR} ${EXECUTABLE_NAME} )
