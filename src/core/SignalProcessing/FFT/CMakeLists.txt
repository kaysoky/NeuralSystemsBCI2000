###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for FFTSignalProcessing in Qt
 
# Set the executable name
SET( EXECUTABLE_NAME FFTSignalProcessing )

# Set the project specific sources
SET( SRC_PROJECT
  FFTSignalProcessing.cpp
  PipeDefinition.cpp 
)
SET( HDR_PROJECT
)

# Create the signal processing module
BCI2000_ADD_SIGNAL_PROCESSING_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
)
