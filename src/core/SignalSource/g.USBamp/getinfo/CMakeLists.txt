###########################################################################
## $Id$
## Author: juergen.mellinger@uni-tuebingen.de
## Description: Build information for USBampgetinfo

# Set the executable name
SET( EXECUTABLE_NAME gUSBampgetinfo )

# DEBUG
MESSAGE( "-- Adding Tool Project: " ${EXECUTABLE_NAME} )

# Set the project specific sources
SET( SRC_PROJECT
  gUSBampgetinfo.cpp
)
SET( HDR_PROJECT
)

# Tell the macro to link against the gUSBamp API found in EXTLIB
BCI2000_INCLUDE( "GUSBAMP" )

# Create the signal source module
ADD_EXECUTABLE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
)

TARGET_LINK_LIBRARIES( ${EXECUTABLE_NAME} ${LIBS} )
