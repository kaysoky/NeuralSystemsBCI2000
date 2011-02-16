###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for FeedbackDemo in Qt

# Set the executable name
SET( EXECUTABLE_NAME FeedbackDemo )

# Set the project specific sources
SET( SRC_PROJECT
  FeedbackDemo.cpp
  FeedbackDemoTask.cpp
)
SET( HDR_PROJECT
  FeedbackDemoTask.h
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
  ${BCI2000_SRC_DIR}/shared/modules/application/
  ${BCI2000_SRC_DIR}/shared/modules/application/utils
)

# Create the signal processing module
BCI2000_ADD_APPLICATION_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
  "${SRC_APPLICATION}" "${HDR_APPLICATION}" 
  "${INCLUDES}"
)


