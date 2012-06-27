###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for CursorTask in Qt

# Set the executable name
SET( EXECUTABLE_NAME CursorTask )

# Set the project specific sources
SET( SRC_PROJECT
  CursorFeedbackTask.cpp
  FeedbackScene2D.cpp
  FeedbackScene3D.cpp
)
SET( HDR_PROJECT
  CursorFeedbackTask.h
  FeedbackScene2D.h
  FeedbackScene3D.h
  FeedbackScene.h
)

# Define extra include directories
INCLUDE_DIRECTORIES(
)

# Create the module
BCI2000_ADD_APPLICATION_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
)


