###########################################################################
## $Id: CMakeLists.txt 3547 2011-09-15 12:54:10Z mellinger $
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for BCI2000RemoteLib

# Set the executable name
SET( NAME BCI2000RemoteLib )

SET( SRC_PROJECT
  BCI2000RemoteLib.cpp
  BCI2000Connection.cpp
  BCI2000Remote.cpp
)

SET( HDR_PROJECT
  BCI2000RemoteLib.h
  BCI2000Connection.h  
  BCI2000Remote.h
)

SET( SRC_FRAMEWORK
  ${BCI2000_SRC_DIR}/shared/utils/SockStream.cpp
)

SET( HDR_FRAMEWORK 
  ${BCI2000_SRC_DIR}/shared/utils/SockStream.h
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
  ${BCI2000_SRC_DIR}/shared/utils/
)

ADD_DEFINITIONS(
  -DLIBREMOTE_LIBRARY=1
)

SET( LIBS "" )
IF( WIN32 )
  SET( LIBS
    ${LIBS}
    ws2_32
  )
ENDIF()

ADD_LIBRARY(
  ${NAME} 
  MODULE
  ${SRC_BCI2000_FRAMEWORK} ${HDR_BCI2000_FRAMEWORK} 
  ${SRC_PROJECT} ${HDR_PROJECT}
)
TARGET_LINK_LIBRARIES( ${NAME} ${LIBS} )
SET_PROPERTY( TARGET "${NAME}" PROPERTY PREFIX "" )
SET_OUTPUT_DIRECTORY( "${BCI2000_ROOT_DIR}/prog" ${NAME} )
# Set the project build folder
SET_PROPERTY( TARGET ${NAME} PROPERTY FOLDER "${DIR_NAME}" )
