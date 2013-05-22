###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for the ParallelSwitch example

IF( BUILD_DEMOS )

IF( WIN32 )

# Set the executable name
SET( NAME ParallelSwitch )

MESSAGE( "-- Adding Example Project: " ${NAME} )

# Set the project specific sources
SET( SRC_PROJECT
  ParallelSwitch.cpp
  ${BCI2000_SRC_DIR}/shared/utils/SockStream.cpp
)

INCLUDE_DIRECTORIES( ${BCI2000_SRC_DIR}/shared/utils )
SET_OUTPUT_DIRECTORY( ${BCI2000_ROOT_DIR}/src/contrib/AppConnectorApplications/ParallelSwitch )
BCI2000_ADD_TARGET( EXECUTABLE ${NAME} ${SRC_PROJECT} )

# Set the target build folder
SET_PROPERTY( TARGET ${NAME} PROPERTY FOLDER Demos/AppConnector )

IF( WIN32 )
  TARGET_LINK_LIBRARIES( ${NAME} ws2_32 )
ENDIF( WIN32 )

ENDIF( WIN32 )

ENDIF( BUILD_DEMOS )