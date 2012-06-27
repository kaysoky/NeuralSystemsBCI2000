###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for FeedbackDemo in Qt

# Set the executable name
SET( EXECUTABLE_NAME FeedbackDemo )

# Set the project specific sources
SET( SRC_PROJECT
  FeedbackDemoTask.cpp
)
SET( HDR_PROJECT
  FeedbackDemoTask.h
)

# Define extra include directories
INCLUDE_DIRECTORIES(
)

# Create the module
BCI2000_ADD_APPLICATION_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
)


