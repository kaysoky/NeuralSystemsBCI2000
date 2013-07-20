###########################################################################
## $Id: CMakeLists.txt 4194 2012-06-27 19:03:26Z mellinger $
## Authors: griffin.milsap@gmail.com
## Description: Build information for gUSBampSource

# Add getinfo here to avoid linker settings defined by BCI2000_ADD_SIGNAL_SOURCE_MODULE
ADD_SUBDIRECTORY( getinfo )

# Set the executable name
SET( EXECUTABLE_NAME gUSBampSource )

# Set the project specific sources
SET( SRC_PROJECT
  gUSBampADC.cpp
  CyberGloveLogger.cpp
)
SET( HDR_PROJECT
  gUSBampADC.h
  CyberGloveLogger.h
)

# Tell the macro to link against the gUSBamp API found in EXTLIB
BCI2000_INCLUDE( "GUSBAMP" )

# Create the signal source module
BCI2000_ADD_SIGNAL_SOURCE_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
)
