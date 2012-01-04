###########################################################################
## $Id:  $
## Authors:
## Description: Build information for the CoherenceFFTSignalProcessing project
##              
##              
###########################################################################

# Set the executable name
SET( EXECUTABLE_NAME CoherenceFFTSignalProcessing )

# Set the project specific sources
SET( SRC_PROJECT
  CoherenceFFTSignalProcessing.cpp            # You probably will not need to edit this file
  PipeDefinition.cpp         # but you should certainly edit this one
  # MyNewCustomFilter.cpp    # and presumably you will be writing custom filters. This is where they go.
  CoherenceFFTFilter.cpp
  ../Coherence/CoherenceFilter.cpp
)

SET( HDR_PROJECT
  # MyNewCustomFilter.h      # Don't forget the corresponding headers
  CoherenceFFTFilter.h
  ../Coherence/CoherenceFilter.h
)

INCLUDE_DIRECTORIES( ../Coherence )

# We're done. Add the signal processing module to the Makefile or compiler project file:
BCI2000_ADD_SIGNAL_PROCESSING_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
)

