###########################################################################
## $Id$
## Authors:
## Description: Build information for DynamicFeedbackTask in Qt

# Set the executable name
SET( EXECUTABLE_NAME Brain2Brain )

# Set the project specific sources
SET( SRC_PROJECT
  # This annoyingly does not get included by default for some projects
  ${PROJECT_SRC_DIR}/shared/modules/CoreMain.cpp
  Brain2Brain.cpp
  Brain2BrainUI.cpp
)
SET( HDR_PROJECT
  Brain2Brain.h
  Brain2BrainUI.h
)

# Define extra include directories
INCLUDE_DIRECTORIES(
)

# Create the module
BCI2000_ADD_APPLICATION_MODULE(
  "${EXECUTABLE_NAME}"
  "${SRC_PROJECT}" "${HDR_PROJECT}"
)


