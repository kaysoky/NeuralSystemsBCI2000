###########################################################################
## $Id: CMakeLists.txt 3547 2011-09-15 12:54:10Z mellinger $
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for BCI2000Shell

# Set the executable name
SET( NAME BCI2000Shell )

INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/Core.cmake )

SET( SRC_PROJECT
  main.cpp
)
SET( HDR_PROJECT
)

SET( SRC_FRAMEWORK
  ${BCI2000_SRC_DIR}/core/Operator/BCI2000Remote/BCI2000Connection.cpp
)

SET( HDR_FRAMEWORK 
  ${BCI2000_SRC_DIR}/core/Operator/BCI2000Remote/BCI2000Connection.h  
)

SOURCE_GROUP( Source\ Files\\BCI2000\ Framework FILES ${SRC_FRAMEWORK} )
SET( SRC_BCI2000_FRAMEWORK
  ${SRC_BCI2000_FRAMEWORK}
  ${SRC_FRAMEWORK}
)

SOURCE_GROUP( Header\ Files\\BCI2000\ Framework FILES ${HDR_FRAMEWORK} )
SET( HDR_BCI2000_FRAMEWORK
  ${HRD_BCI2000_FRAMEWORK}
  ${HDR_FRAMEWORK}
)

INCLUDE_DIRECTORIES(
  ${BCI2000_SRC_DIR}/core/Operator/BCI2000Remote
)

BCI2000_SETUP_EXTLIB_DEPENDENCIES( SRC_BCI2000_FRAMEWORK HDR_BCI2000_FRAMEWORK LIBS FAILED )
IF( WIN32 )
  SET( LIBS
    ${LIBS}
  )
ENDIF( WIN32 )

IF( NOT FAILED )
  ADD_EXECUTABLE(
    ${NAME}
    ${SRC_BCI2000_FRAMEWORK}
    ${HDR_BCI2000_FRAMEWORK}
    ${SRC_PROJECT}
    ${HDR_PROJECT}
  )
  TARGET_LINK_LIBRARIES( ${NAME} ${LIBS} )
  SET_OUTPUT_DIRECTORY( "${BCI2000_ROOT_DIR}/prog" ${NAME} )
  # Set the project build folder
  SET_PROPERTY( TARGET ${NAME} PROPERTY FOLDER "${DIR_NAME}" )

ENDIF()
