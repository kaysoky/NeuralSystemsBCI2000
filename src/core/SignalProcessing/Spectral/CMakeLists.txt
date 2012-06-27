###########################################################################
## $Id: CMakeLists.txt 3497 2011-08-30 16:50:58Z mellinger $
## Authors: griffin.milsap@gmail.com
## Description: Build information for SpectralSignalProcessing in Qt

# Set the executable name
SET( EXECUTABLE_NAME SpectralSignalProcessing )

# Set the project specific sources
SET( SRC_PROJECT
  PipeDefinition.cpp 
)
SET( HDR_PROJECT
)

BCI2000_USE( "FFT" )

# Create the signal processing module
BCI2000_ADD_SIGNAL_PROCESSING_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
)


