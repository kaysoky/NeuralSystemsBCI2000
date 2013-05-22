###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for BCI2000PresentationLink

IF( WIN32 )

SET( NAME BCI2000PresentationLink )
IF( CMAKE_SIZEOF_VOID_P EQUAL 8 )
  SET( EXECUTABLE_NAME "${EXECUTABLE_NAME}64" )
ENDIF()

MESSAGE( "-- Adding Project: ${NAME}" )

SET( SOURCES
  BCI2000PresentationLink.cpp
  BCI2000DataPortExtension.cpp
  BCI2000DataPort.cpp
  BCI2000ActiveDataPort.cpp
  BCI2000PCLLibrary.cpp
  BCI2000ControllerType.cpp
  BCI2000Controller.cpp
  Utils.cpp
  BCI2000PresentationLink_i.c
  BCI2000PresentationLink.rc
  Resource.h

  ${BCI2000_SRC_DIR}/shared/config/Version.h
  ${BCI2000_SRC_DIR}/shared/bcistream/BCIException.h

  ${BCI2000_SRC_DIR}/shared/utils/VersionInfo.cpp
  ${BCI2000_SRC_DIR}/shared/utils/SockStream.cpp
  ${BCI2000_SRC_DIR}/core/Operator/BCI2000Remote/BCI2000Remote.cpp  
  ${BCI2000_SRC_DIR}/core/Operator/BCI2000Remote/BCI2000Connection.cpp  

  ${BCI2000_SRC_DIR}/extlib/nbs/DataPortExtension/interfaces/DataPortExtension_i.c
  ${BCI2000_SRC_DIR}/extlib/nbs/PCLExtension/interface/PCLExtension_i.c
  ${BCI2000_SRC_DIR}/extlib/nbs/DataPortExtension/interfaces/DataPortExtension.h
  ${BCI2000_SRC_DIR}/extlib/nbs/PCLExtension/interface/PCLExtension.h
)

BCI2000_INCLUDE( COM )
BCI2000_ADD_TARGET( MODULE ${NAME} ${SOURCES} )
ADD_CUSTOM_COMMAND(
  TARGET ${NAME}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/InstallPresentationLink.cmd" "${BCI2000_ROOT_DIR}/prog"
)

ENDIF( WIN32 )
