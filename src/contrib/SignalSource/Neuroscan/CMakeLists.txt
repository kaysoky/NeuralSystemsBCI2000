###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for Neuroscan source

ADD_SUBDIRECTORY( getparams )

# Set the executable name
SET( EXECUTABLE_NAME NeuroscanClient )

# Set the project specific sources
SET( SRC_PROJECT
  NeuroscanADC.cpp 
)
SET( HDR_PROJECT
  NeuroscanNetRead.h
  NeuroscanADC.h
)

# Create the signal source module
BCI2000_ADD_SIGNAL_SOURCE_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
)
