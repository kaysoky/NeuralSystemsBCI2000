###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Sets up CMAKE variables for including the 3D API in a project
## SETS:
##       SRC_EXTLIB - Required source files for the 3D API
##       HDR_EXTLIB - Required header files for the 3D API
##       INC_EXTLIB - Include directory for the 3D API
##       LIBS_EXTLIB - Required libraries for the 3DAPI (OpenGL)
##       Also defines source groups for the 3D API files

# Let CMake know we plan to link against the Qt OPENGL libraries
SET( QT_USE_QTOPENGL TRUE )
INCLUDE(${QT_USE_FILE})
SET( LIBS_EXTLIB
  ${QT_LIBRARIES}
)
FIND_PACKAGE( OpenGL REQUIRED )
IF( OPENGL_FOUND )
  SET( LIBS_EXTLIB
    ${LIBS_EXTLIB}
    ${OPENGL_LIBRARIES}
  )
ELSE( OPENGL_FOUND )
  MESSAGE( FATAL_ERROR "OpenGL package not found." )
ENDIF( OPENGL_FOUND )

# Define the source files
SET( SRC_EXTLIB
  ${BCI2000_SRC_DIR}/extlib/3DAPI/buffers.cpp
  ${BCI2000_SRC_DIR}/extlib/3DAPI/cameraNlight.cpp
  ${BCI2000_SRC_DIR}/extlib/3DAPI/cuboids.cpp
  ${BCI2000_SRC_DIR}/extlib/3DAPI/geomObj.cpp
  ${BCI2000_SRC_DIR}/extlib/3DAPI/halfSpace.cpp
  ${BCI2000_SRC_DIR}/extlib/3DAPI/Load3DS.cpp
  ${BCI2000_SRC_DIR}/extlib/3DAPI/model3D.cpp
  ${BCI2000_SRC_DIR}/extlib/3DAPI/primObj.cpp
  ${BCI2000_SRC_DIR}/extlib/3DAPI/Scene.cpp
  ${BCI2000_SRC_DIR}/extlib/3DAPI/sphere.cpp
  ${BCI2000_SRC_DIR}/extlib/3DAPI/threeDText.cpp
  ${BCI2000_SRC_DIR}/extlib/3DAPI/twoDCursor.cpp
  ${BCI2000_SRC_DIR}/extlib/3DAPI/twoDOverlay.cpp
  ${BCI2000_SRC_DIR}/extlib/3DAPI/twoDText.cpp
)

# Define the headers
SET( HDR_EXTLIB
  ${BCI2000_SRC_DIR}/extlib/3DAPI/buffers.h
  ${BCI2000_SRC_DIR}/extlib/3DAPI/cameraNlight.h
  ${BCI2000_SRC_DIR}/extlib/3DAPI/component.h
  ${BCI2000_SRC_DIR}/extlib/3DAPI/cuboids.h
  ${BCI2000_SRC_DIR}/extlib/3DAPI/geomObj.h
  ${BCI2000_SRC_DIR}/extlib/3DAPI/glheaders.h
  ${BCI2000_SRC_DIR}/extlib/3DAPI/halfSpace.h
  ${BCI2000_SRC_DIR}/extlib/3DAPI/Load3DS.h
  ${BCI2000_SRC_DIR}/extlib/3DAPI/model3D.h
  ${BCI2000_SRC_DIR}/extlib/3DAPI/primObj.h
  ${BCI2000_SRC_DIR}/extlib/3DAPI/Scene.h
  ${BCI2000_SRC_DIR}/extlib/3DAPI/sphere.h
  ${BCI2000_SRC_DIR}/extlib/3DAPI/threeDText.h
  ${BCI2000_SRC_DIR}/extlib/3DAPI/twoDCursor.h
  ${BCI2000_SRC_DIR}/extlib/3DAPI/twoDOverlay.h
  ${BCI2000_SRC_DIR}/extlib/3DAPI/twoDText.h
)

# Define the include directory
SET( INC_EXTLIB ${BCI2000_SRC_DIR}/extlib/3DAPI )

# Set the source groups
SOURCE_GROUP( Source\\BCI2000_Framework\\extlib\\3DAPI FILES ${SRC_EXTLIB} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\extlib\\3DAPI FILES ${HDR_EXTLIB} )

# Set success
SET( EXTLIB_OK TRUE )
