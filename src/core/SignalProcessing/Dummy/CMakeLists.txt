###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for a Dummy signal processing module

# Set the executable name 
SET( EXECUTABLE_NAME DummySignalProcessing )

# Set the project specific sources 
SET( SRC_PROJECT
  DummySignalProcessing.cpp
  PipeDefinition.cpp 
)
SET( HDR_PROJECT

)

# Set required signal processing source files
SET( SRC_SIGNALPROCESSING

)
SET( HDR_SIGNALPROCESSING 

)

# Define include directories
SET( INC_EXTLIB

)

# Set extra required source files
SET( SRC_EXTRA

)
SET( HDR_EXTRA

)

# Create the signal processing module
BCI2000_ADD_SIGNAL_PROCESSING_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
  "${SRC_SIGNALPROCESSING}" "${HDR_SIGNALPROCESSING}" 
  "${INC_EXTLIB}" "${SRC_EXTRA}" "${HDR_EXTRA}"
)
