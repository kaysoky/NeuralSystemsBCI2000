###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for gUSBampSource

# Set the executable name
SET( EXECUTABLE_NAME gUSBampSource )

# Set the project specific sources
SET( SRC_PROJECT
  gUSBampSource.cpp
  gUSBampADC.cpp 
)
SET( HDR_PROJECT
  gUSBampADC.h
)

# Tell the macro to link against the gUSBamp API found in EXTLIB
BCI2000_USE( "GUSBAMP" )

# Create the signal source module
BCI2000_ADD_SIGNAL_SOURCE_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
  "${INC_EXTLIB}"
)
