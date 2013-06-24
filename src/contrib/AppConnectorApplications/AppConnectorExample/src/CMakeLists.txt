###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for the AppConnectorExample

IF( BUILD_DEMOS )

# Set the executable name
SET( NAME AppConnectorExample )

# Set the project specific sources
SET( SRC_PROJECT
  main.cpp
  mainUI.cpp
  ${BCI2000_SRC_DIR}/shared/utils/SockStream.cpp
)

SET_OUTPUT_DIRECTORY( ${BCI2000_ROOT_DIR}/src/contrib/AppConnectorApplications/AppConnectorExample )
BCI2000_ADD_TARGET( QTAPP ${NAME} ${SRC_PROJECT} ${HDR_PROJECT} )
SET_PROPERTY( TARGET ${NAME} PROPERTY FOLDER Demos/AppConnector )
IF( WIN32 )
  TARGET_LINK_LIBRARIES( ${NAME} ws2_32 )
ENDIF( WIN32 )

ENDIF( BUILD_DEMOS )
