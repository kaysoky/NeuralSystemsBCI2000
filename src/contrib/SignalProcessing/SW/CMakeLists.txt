###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for SWSignalProcessing module

# Set the executable name
SET( EXECUTABLE_NAME SWSignalProcessing )

# Set the project specific sources
SET( SRC_PROJECT
  SWSignalProcessing.cpp
  FBArteCorrection.cpp
  SetBaseline.cpp
  SWFilter.cpp
  PipeDefinition.cpp
)
SET( HDR_PROJECT
  FBArteCorrection.h
  SetBaseline.h
  SWFilter.h
)

# Create the signal processing module
BCI2000_ADD_SIGNAL_PROCESSING_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
)
