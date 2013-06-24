###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for the TDTSampleRate tool

# Set the executable name
SET( NAME TDTSampleRate )

SET( SOURCES
  TDTSampleRate.cpp
  MainDialog.cpp
)

BCI2000_ADD_TARGET( INFO Tool QTAPP ${NAME} ${SOURCES} )
BCI2000_ADD_TO_INVENTORY( Tool ${NAME} )
