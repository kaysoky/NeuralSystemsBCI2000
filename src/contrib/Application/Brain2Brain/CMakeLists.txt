###########################################################################
## $Id$
## Authors: 
## Description: Build information for DynamicFeedbackTask in Qt

# Set the executable name
SET( EXECUTABLE_NAME Brain2Brain )

# Set the project specific sources
SET( SRC_PROJECT
  B2B.cpp
  DFBuildScene2D.cpp
  GradientEllipticShape.cpp 
  mongoose.c
)
SET( HDR_PROJECT
  B2B.h
  DFBuildScene2D.h
  DFBuildScene.h
  GradientEllipticShape.h
  mongoose.h
)

# Define extra include directories
INCLUDE_DIRECTORIES(
)

# Create the module
BCI2000_ADD_APPLICATION_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
)


