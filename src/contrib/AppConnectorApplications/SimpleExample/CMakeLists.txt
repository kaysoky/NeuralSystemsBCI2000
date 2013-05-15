###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for the ParallelSwitch example

IF( BUILD_DEMOS )

# Set the executable name
SET( NAME SimpleExample )

MESSAGE( "-- Adding Example Project: " ${NAME} )

# Set the project specific sources
SET( SRC_PROJECT
  SimpleExample.cpp
  ${BCI2000_SRC_DIR}/shared/utils/SockStream.cpp
)
SET( HDR_PROJECT
  ${BCI2000_SRC_DIR}/shared/utils/SockStream.h
)

SOURCE_GROUP( Source\\Project FILES
  ParallelSwitch.cpp
)
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\utils FILES 
  ${BCI2000_SRC_DIR}/shared/utils/SockStream.cpp
)
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\utils FILES 
  ${BCI2000_SRC_DIR}/shared/utils/SockStream.h
)

INCLUDE_DIRECTORIES( ${BCI2000_SRC_DIR}/shared/utils )
SET_OUTPUT_DIRECTORY( ${BCI2000_ROOT_DIR}/src/contrib/AppConnectorApplications/SimpleExample )
ADD_EXECUTABLE( ${NAME} ${SRC_PROJECT} ${HDR_PROJECT} )

IF( WIN32 )
  TARGET_LINK_LIBRARIES( ${NAME} ws2_32 )
ENDIF( WIN32 )

# Set the project build folder
SET_PROPERTY( TARGET ${NAME} PROPERTY FOLDER Demos/AppConnector )

ENDIF( BUILD_DEMOS )
