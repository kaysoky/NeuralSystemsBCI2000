###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for the AppConnectorExample

# Set the executable name
SET( NAME AppConnectorExample )

IF( NOT BORLAND )

MESSAGE( "-- Adding Example Project: " ${NAME} )

# Set the project specific sources
SET( SRC_PROJECT
  main.cpp
  mainUI.cpp
  ${BCI2000_SRC_DIR}/shared/utils/SockStream.cpp
)
SET( HDR_PROJECT
  mainUI.h
  ${BCI2000_SRC_DIR}/shared/utils/SockStream.h
)

INCLUDE_DIRECTORIES( ${BCI2000_SRC_DIR}/shared/utils )

QT_WRAP_CPP( ${NAME} GENERATED mainUI.h )
SOURCE_GROUP( Generated FILES ${GENERATED} )
SET( SRC_PROJECT
  ${SRC_PROJECT}
  ${GENERATED}
)

SET_OUTPUT_DIRECTORY( ${BCI2000_ROOT_DIR}/src/contrib/AppConnectorApplications/AppConnectorExample )
ADD_EXECUTABLE( ${NAME} WIN32 ${SRC_PROJECT} ${HDR_PROJECT} )
SET_PROPERTY( TARGET ${NAME} APPEND PROPERTY COMPILE_FLAGS "-DUSE_QT" )

TARGET_LINK_LIBRARIES( ${NAME} ${QT_LIBRARIES} )
IF( WIN32 )
  TARGET_LINK_LIBRARIES( ${NAME} ws2_32 )
ENDIF( WIN32 )

ENDIF( NOT BORLAND )
