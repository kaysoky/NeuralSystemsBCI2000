###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for the TDTSampleRate tool

# Set the executable name
SET( NAME TDTSampleRate )

MESSAGE( "-- Adding Tool Project: " ${NAME} )

SET( SOURCES
  TDTSampleRate.cpp
  MainDialog.cpp
)

BCI2000_ADD_TARGET( QTAPP ${NAME} ${SOURCES} )
BCI2000_ADD_TO_INVENTORY( Tool ${NAME} )
