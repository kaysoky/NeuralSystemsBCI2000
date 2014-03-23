###########################################################################
## $Id$
## Authors: 
## Description: Build information for DynamicFeedbackTask in Qt

# Set the executable name
SET( EXECUTABLE_NAME SSVEP )

# Set the project specific sources
SET( SRC_PROJECT
  # This annoyingly does not get included by default for some projects
  ${PROJECT_SRC_DIR}/shared/modules/CoreMain.cpp
  SSVEP.cpp
  SSVEPUI.cpp
  RadialArrow.cpp
)
SET( HDR_PROJECT
  SSVEP.h
  SSVEPUI.h
  RadialArrow.h
)

# Define extra include directories
INCLUDE_DIRECTORIES(
)

# Create the module
BCI2000_ADD_APPLICATION_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
)


