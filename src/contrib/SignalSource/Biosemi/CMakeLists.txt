###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for Biosemi2 source module

# Set the executable name
SET( EXECUTABLE_NAME Biosemi2 )

# Set the project specific sources
SET( SRC_PROJECT
  Biosemi2ADC.cpp
  Biosemi2Client.cpp
)
SET( HDR_PROJECT
  Biosemi2ADC.h
  Biosemi2Client.h
)

BCI2000_INCLUDE( SOURCEFILTER )

# Create the signal source module
BCI2000_ADD_SIGNAL_SOURCE_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
)

ADD_CUSTOM_COMMAND(
  TARGET "${EXECUTABLE_NAME}"
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/Labview_DLL.dll" "${BCI2000_ROOT_DIR}/prog"
)
