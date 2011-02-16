###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for SignalGenerator in Qt

# Set the executable name
SET( EXECUTABLE_NAME SignalGenerator )

# Set the project specific sources
SET( SRC_PROJECT
  SignalGenerator.cpp
  SignalGeneratorADC.cpp 
)
SET( HDR_PROJECT
  SignalGeneratorADC.h
)

# Let the macro know we want to use the Math library in EXTLIB
BCI2000_USE( "MATH" )

# Create the signal source module
BCI2000_ADD_SIGNAL_SOURCE_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
  "${INC_EXTLIB}"
)
