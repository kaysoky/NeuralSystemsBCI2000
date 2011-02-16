###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for CursorTask in Qt

# Set the executable name
SET( EXECUTABLE_NAME CursorTask )

# Set the project specific sources
SET( SRC_PROJECT
  CursorTask.cpp
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

# Set required application source files
SET( SRC_APPLICATION
  ${BCI2000_SRC_DIR}/shared/modules/application/FeedbackTask.cpp
)
SET( HDR_APPLICATION
  ${BCI2000_SRC_DIR}/shared/modules/application/FeedbackTask.h
)

SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\modules\\application FILES ${SRC_APPLICATION} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\modules\\application FILES ${HDR_APPLICATION} )

# Define extra include directories
SET( INCLUDES
  ${BCI2000_SRC_DIR}/shared/modules/application
  ${BCI2000_SRC_DIR}/shared/modules/application/utils
)

# Let the macro know we want to include the 3D API
BCI2000_USE( "3DAPI" )

# Create the signal processing module
BCI2000_ADD_APPLICATION_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
  "${SRC_APPLICATION}" "${HDR_APPLICATION}" 
  "${INCLUDES}"
)


