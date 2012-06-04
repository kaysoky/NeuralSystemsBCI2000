###########################################################################
## $Id$
## Author: juergen.mellinger@uni-tuebingen.de
## Description: Build information for PythonApplication

# Set the executable name
SET( EXECUTABLE_NAME PythonApplication )

# Set the project specific sources
SET( SRC_PROJECT
  PythonApplication.cpp
  ../shared/PythonFilter.cpp
  ../shared/PythonLink24.cpp
  ../shared/PythonLink25.cpp
  ../shared/PythonLink26.cpp
  ../shared/PythonLink27.cpp
  ../shared/PythonWrapper.cpp
)
SET( HDR_PROJECT
  ../shared/PythonFilter.h
  ../shared/PythonWrapper.h
)

ADD_DEFINITIONS(
  -DDYNAMIC_PYTHON=1
)

# Create the signal processing module
BCI2000_ADD_APPLICATION_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
)

ADD_CUSTOM_COMMAND(
  TARGET "${EXECUTABLE_NAME}"
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${EXECUTABLE_NAME}${CMAKE_EXECUTABLE_SUFFIX}" "${CMAKE_CURRENT_SOURCE_DIR}/../../demo/prog"
)
