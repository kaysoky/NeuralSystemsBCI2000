###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for BCI2000Command

# Set the executable name
SET( EXECUTABLE_NAME BCI2000Command )

ADD_DEFINITIONS(
  -DEXECUTABLE_NAME=\"${EXECUTABLE_NAME}\"
  -DOPERATOR_NAME=\"Operator\"
)

# Set the project specific sources
SET( SRC_PROJECT
  BCI2000Command.cpp
  ${BCI2000_SRC_DIR}/shared/utils/SockStream.cpp
  ${BCI2000_SRC_DIR}/core/Operator/BCI2000Remote/BCI2000Remote.cpp  
  ${BCI2000_SRC_DIR}/core/Operator/BCI2000Remote/BCI2000Connection.cpp  
)

IF( WIN32 )
  SET( LIBS
    ${LIBS}
    ws2_32
    shlwapi
  )
ENDIF( WIN32 )

BCI2000_ADD_TARGET( EXECUTABLE ${EXECUTABLE_NAME} ${SRC_PROJECT} )
