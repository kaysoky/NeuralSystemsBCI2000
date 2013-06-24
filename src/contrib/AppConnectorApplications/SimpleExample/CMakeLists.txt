###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for the ParallelSwitch example

IF( BUILD_DEMOS )

# Set the executable name
SET( NAME SimpleExample )

# Set the project specific sources
SET( SRC_PROJECT
  SimpleExample.cpp
  ${BCI2000_SRC_DIR}/shared/utils/SockStream.cpp
)

SET_OUTPUT_DIRECTORY( ${BCI2000_ROOT_DIR}/src/contrib/AppConnectorApplications/SimpleExample )
BCI2000_ADD_TARGET( INFO "AppConnector example" EXECUTABLE ${NAME} ${SRC_PROJECT} )

IF( WIN32 )
  TARGET_LINK_LIBRARIES( ${NAME} ws2_32 )
ENDIF( WIN32 )

# Set the project build folder
SET_PROPERTY( TARGET ${NAME} PROPERTY FOLDER Demos/AppConnector )

ENDIF( BUILD_DEMOS )