###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for StimulusPresentation in Qt

# Set the executable name
SET( EXECUTABLE_NAME StimulusPresentation )

# Set the project specific sources
SET( SRC_PROJECT
  StimulusPresentation.cpp
  StimulusPresentationTask.cpp
)
SET( HDR_PROJECT
  StimulusPresentationTask.h
)

# Set required application source files
SET( SRC_APPLICATION
)
SET( HDR_APPLICATION
)

# Define extra include directories
INCLUDE_DIRECTORIES(
)

# Create the module
BCI2000_ADD_APPLICATION_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
  "${SRC_APPLICATION}" "${HDR_APPLICATION}" 
)


