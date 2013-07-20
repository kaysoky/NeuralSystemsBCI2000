###########################################################################
## $Id: CMakeLists.txt 3577 2011-09-30 16:18:48Z mellinger $
## Authors: griffin.milsap@gmail.com
## Description: Build information for DynamicFeedbackTask in Qt

# Set the executable name
SET( EXECUTABLE_NAME B2B )

# Set the project specific sources
SET( SRC_PROJECT
  B2B.cpp
  DFBuildScene2D.cpp
  GradientEllipticShape.cpp 
)
SET( HDR_PROJECT
  B2B.h
  DFBuildScene2D.h
  DFBuildScene.h
  GradientEllipticShape.h
)

# Define extra include directories
INCLUDE_DIRECTORIES(
)

# Create the module
BCI2000_ADD_APPLICATION_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
)


