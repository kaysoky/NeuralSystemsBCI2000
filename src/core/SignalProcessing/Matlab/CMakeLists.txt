###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for MatlabSignalProcessing in Qt

# Set the executable name
SET( EXECUTABLE_NAME MatlabSignalProcessing )

# Set the project specific sources
SET( SRC_PROJECT
  PipeDefinition.cpp 
  MatlabFilter.cpp
  MatlabWrapper.cpp
)
SET( HDR_PROJECT
  MatlabFilter.h
  MatlabWrapper.h
)

# Make sure the macro knows we want to use EXTLIB libs!
BCI2000_INCLUDE( "MATLAB" )

# Create the signal processing module
BCI2000_ADD_SIGNAL_PROCESSING_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
)
