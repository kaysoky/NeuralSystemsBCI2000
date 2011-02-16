###########################################################################
## $Id$
## Author: juergen.mellinger@uni-tuebingen.de
## Description: Build information for PythonSource

# Set the executable name
SET( EXECUTABLE_NAME PythonSource )

# Set the project specific sources
SET( SRC_PROJECT
  PythonSource.cpp
  ../shared/PythonFilter.cpp
  ../shared/PythonLink24.cpp
  ../shared/PythonLink25.cpp
  ../shared/PythonLink26.cpp
  ../shared/PythonWrapper.cpp
)
SET( HDR_PROJECT
  ../shared/PythonFilter.h
  ../shared/PythonWrapper.h
)

# Set required application source files
SET( SRC_SIGNALSOURCE
)
SET( HDR_SIGNALSOURCE
)

# Define extra include directories
SET( INCLUDES
)

ADD_DEFINITIONS(
  -DDYNAMIC_PYTHON=1
)

# Create the signal processing module
BCI2000_ADD_SIGNAL_SOURCE_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
  "${SRC_SIGNALSOURCE}" "${HDR_SIGNALSOURCE}" 
  "${INCLUDES}"
)

ADD_CUSTOM_COMMAND(
  TARGET "${EXECUTABLE_NAME}"
  COMMAND ${CMAKE_COMMAND} -E copy "${BCI2000_ROOT_DIR}/prog/PythonSource.exe" "${CMAKE_CURRENT_SRC_DIR}/../../demo/prog"
)
