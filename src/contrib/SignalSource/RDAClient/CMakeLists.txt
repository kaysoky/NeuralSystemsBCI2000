###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for RDAClient source

ADD_SUBDIRECTORY( getparams )

# Set the executable name
SET( EXECUTABLE_NAME RDAClient )

# Set the project specific sources
SET( SRC_PROJECT
  RDAClientADC.cpp 
  RDAQueue.cpp
)
SET( HDR_PROJECT
  RDA/RecorderRDA.h
  RDAClientADC.h
  RDAQueue.h
)

# Create the signal source module
BCI2000_ADD_SIGNAL_SOURCE_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
)
