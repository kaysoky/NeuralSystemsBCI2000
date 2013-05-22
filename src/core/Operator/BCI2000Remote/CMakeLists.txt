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
  ${BCI2000_SRC_DIR}/shared/utils/SockStream.cpp
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

BCI2000_ADD_TARGET( DLL ${NAME} ${SRC_PROJECT} )
SET_OUTPUT_DIRECTORY( "${BCI2000_ROOT_DIR}/prog" ${NAME} )
