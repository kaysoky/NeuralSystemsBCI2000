###########################################################################
## $Id$
## Authors: 
## Description: Build information for DynamicFeedbackTask in Qt

# Set the executable name
SET( EXECUTABLE_NAME SSVEP )

# Set the project specific sources
SET( SRC_PROJECT
  SSVEP.cpp
)
SET( HDR_PROJECT
  SSVEP.h
)

# Define extra include directories
INCLUDE_DIRECTORIES(
)

# Create the module
BCI2000_ADD_APPLICATION_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
)


