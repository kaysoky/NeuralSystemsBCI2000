###########################################################################
## $Id: CMakeLists.txt 3497 2011-08-30 16:50:58Z mellinger $
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for Neuroscan getparams tool

SET( NAME RDAgetparams )

SET( SOURCES
  RDAgetparams.cpp
  ${BCI2000_SRC_DIR}/contrib/SignalSource/RDAClient/RDAProtocol.cpp
  ${BCI2000_SRC_DIR}/shared/bcistream/BCIStream_tool.cpp
)

INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/Core.cmake )
BCI2000_ADD_TARGET( INFO Tool EXECUTABLE ${NAME} ${SOURCES} )
BCI2000_ADD_TO_INVENTORY( Tool ${NAME} )
