###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for gMOBIlabPlus

# Set the executable name
SET( EXECUTABLE_NAME gMOBIlabPlus )

# Set the project specific sources
SET( SRC_PROJECT
  gMOBIlabPlus.cpp
  gMOBIlabPlusADC.cpp 
)
SET( HDR_PROJECT
  gMOBIlabPlusADC.h
  gMOBIlabDriver.h
)

# Tell the macro to link against the gMOBIlabPlus API found in EXTLIB
BCI2000_USE( "GMOBILABPLUS" )

# Create the signal source module
BCI2000_ADD_SIGNAL_SOURCE_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
  "${INC_EXTLIB}"
)
