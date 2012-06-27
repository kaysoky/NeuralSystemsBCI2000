###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for P3Speller in Qt

# Set the executable name
SET( EXECUTABLE_NAME P3Speller )

# Set the project specific sources
SET( SRC_PROJECT
  P3SpellerTask.cpp
)
SET( HDR_PROJECT
  P3SpellerTask.h
)

# Define extra include directories
INCLUDE_DIRECTORIES(
)

# Create the module
BCI2000_ADD_APPLICATION_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
)
