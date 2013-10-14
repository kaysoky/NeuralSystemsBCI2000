###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Sets up CMAKE variables for including the 3D API in a project
## SETS:
##       SRC_EXTLIB - Required source files for the 3D API
##       HDR_EXTLIB - Required header files for the 3D API
##       INC_EXTLIB - Include directory for the 3D API
##       LIBS_EXTLIB - Required libraries for the 3DAPI (OpenGL)

# Let CMake know we plan to link against the Qt OPENGL libraries
IF( QT_IS4 )
  SET( QT_USE_QTOPENGL TRUE )
  INCLUDE(${QT_USE_FILE})
  SET( LIBS_EXTLIB
    ${QT_LIBRARIES}
  )
ELSE()
  SET( QT5_MODULES
    ${QT5_MODULES}
    OpenGL
  )
ENDIF()

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
  ${PROJECT_SRC_DIR}/extlib/3DAPI/buffers.cpp
  ${PROJECT_SRC_DIR}/extlib/3DAPI/cameraNlight.cpp
  ${PROJECT_SRC_DIR}/extlib/3DAPI/cuboids.cpp
  ${PROJECT_SRC_DIR}/extlib/3DAPI/geomObj.cpp
  ${PROJECT_SRC_DIR}/extlib/3DAPI/halfSpace.cpp
  ${PROJECT_SRC_DIR}/extlib/3DAPI/Load3DS.cpp
  ${PROJECT_SRC_DIR}/extlib/3DAPI/model3D.cpp
  ${PROJECT_SRC_DIR}/extlib/3DAPI/primObj.cpp
  ${PROJECT_SRC_DIR}/extlib/3DAPI/Scene.cpp
  ${PROJECT_SRC_DIR}/extlib/3DAPI/sphere.cpp
  ${PROJECT_SRC_DIR}/extlib/3DAPI/threeDText.cpp
  ${PROJECT_SRC_DIR}/extlib/3DAPI/twoDCursor.cpp
  ${PROJECT_SRC_DIR}/extlib/3DAPI/twoDOverlay.cpp
  ${PROJECT_SRC_DIR}/extlib/3DAPI/twoDText.cpp
)

# Define the headers
SET( HDR_EXTLIB
  ${PROJECT_SRC_DIR}/extlib/3DAPI/buffers.h
  ${PROJECT_SRC_DIR}/extlib/3DAPI/cameraNlight.h
  ${PROJECT_SRC_DIR}/extlib/3DAPI/component.h
  ${PROJECT_SRC_DIR}/extlib/3DAPI/cuboids.h
  ${PROJECT_SRC_DIR}/extlib/3DAPI/geomObj.h
  ${PROJECT_SRC_DIR}/extlib/3DAPI/glheaders.h
  ${PROJECT_SRC_DIR}/extlib/3DAPI/halfSpace.h
  ${PROJECT_SRC_DIR}/extlib/3DAPI/Load3DS.h
  ${PROJECT_SRC_DIR}/extlib/3DAPI/model3D.h
  ${PROJECT_SRC_DIR}/extlib/3DAPI/primObj.h
  ${PROJECT_SRC_DIR}/extlib/3DAPI/Scene.h
  ${PROJECT_SRC_DIR}/extlib/3DAPI/sphere.h
  ${PROJECT_SRC_DIR}/extlib/3DAPI/threeDText.h
  ${PROJECT_SRC_DIR}/extlib/3DAPI/twoDCursor.h
  ${PROJECT_SRC_DIR}/extlib/3DAPI/twoDOverlay.h
  ${PROJECT_SRC_DIR}/extlib/3DAPI/twoDText.h
)

# Define the include directory
SET( INC_EXTLIB ${PROJECT_SRC_DIR}/extlib/3DAPI )

# Set success
SET( EXTLIB_OK TRUE )
