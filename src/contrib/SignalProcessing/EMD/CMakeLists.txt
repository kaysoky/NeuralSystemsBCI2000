###########################################################################
## $Id: CMakeLists.txt 4194 2012-06-27 19:03:26Z mellinger $
## Authors: kaysoky@cs.washington.edu
## Description: Build information for a Empirical Mode Decomposition (EMD) based filter

# Set the executable name 
SET( EXECUTABLE_NAME EMDSignalProcessing )

# Set the project specific sources 
SET( SRC_PROJECT
  PipeDefinition.cpp 
  EMDFilter.cpp
  EMDThread.cpp
  InstantFrequencyFilter.cpp
  PairedChannelDiffFilter.cpp
)
SET( HDR_PROJECT
  EMDFilter.h
  EMDThread.h
  InstantFrequencyFilter.h
  PairedChannelDiffFilter.h
)

BCI2000_INCLUDE( "FFT" )

# Create the signal processing module
BCI2000_ADD_SIGNAL_PROCESSING_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
)
