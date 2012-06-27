###########################################################################
## $Id$
## Author: juergen.mellinger@uni-tuebingen.de
## Description: Build information for PythonSource

# Set the executable name
SET( EXECUTABLE_NAME PythonSource )

# Create the signal processing module
BCI2000_ADD_SIGNAL_SOURCE_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
)
TARGET_LINK_LIBRARIES( ${EXECUTABLE_NAME} ${LIBS} )

ADD_CUSTOM_COMMAND(
  TARGET "${EXECUTABLE_NAME}"
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${EXECUTABLE_NAME}${CMAKE_EXECUTABLE_SUFFIX}" "${CMAKE_CURRENT_SOURCE_DIR}/../../demo/prog"
)
