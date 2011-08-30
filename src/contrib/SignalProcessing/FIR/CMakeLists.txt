###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for FIRProcessing module

# Set the executable name
SET( EXECUTABLE_NAME FIRProcessing )

# Set the project specific sources
SET( SRC_PROJECT
  FIRProcessing.cpp
  FIRFilter.cpp
  PipeDefinition.cpp
)
SET( HDR_PROJECT
  FIRFilter.h
)

# Create the signal processing module
BCI2000_ADD_SIGNAL_PROCESSING_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
)
