###########################################################################
## $Id$
## Description: Build information for FieldTripBufferSource

# Set the executable name
SET( EXECUTABLE_NAME FieldTripBufferSource )

# Set the project specific sources
SET( SRC_PROJECT
  FieldTripBufferSource.cpp
  FieldTripBufferADC.cpp 
)
SET( HDR_PROJECT
  FieldTripBufferADC.h
)


BCI2000_USE( "FIELDTRIP" )
IF( WIN32 )
  BCI2000_USE( "PTHREADS" )
ENDIF( WIN32 )

# Create the signal source module
BCI2000_ADD_SIGNAL_SOURCE_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
  "${INC_EXTLIB}"
)
