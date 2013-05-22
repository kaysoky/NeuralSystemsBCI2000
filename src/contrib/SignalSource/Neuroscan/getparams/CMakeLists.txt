###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for Neuroscan getparams tool

# Set the executable name
SET( NAME neurogetparams )

# DEBUG
MESSAGE( "-- Adding Tool Project: " ${NAME} )

# Set the project specific sources
SET( SOURCES
  neuroscanget.cpp
  ${BCI2000_SRC_DIR}/shared/bcistream/BCIStream_tool.cpp
  ${BCI2000_SRC_DIR}/contrib/SignalSource/Neuroscan/NeuroscanProtocol.h
)

INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/Core.cmake )
BCI2000_ADD_TARGET( EXECUTABLE ${NAME} ${SOURCES} )
BCI2000_ADD_TO_INVENTORY( Tool ${NAME} )
