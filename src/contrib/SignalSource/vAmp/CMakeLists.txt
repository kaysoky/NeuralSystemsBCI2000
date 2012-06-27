###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for vAmpSource

# Set the executable name
SET( EXECUTABLE_NAME vAmpSource )

# Set the project specific sources
SET( SRC_PROJECT
  vAmpADC.cpp 
  vAmpChannelInfo.cpp
  vAmpThread.cpp
  vAmpDisplay.cpp
)
SET( HDR_PROJECT
  vAmpADC.h
  vAmpChannelInfo.h
  vAmpThread.h
  vAmpDisplay.h
  ${BCI2000_SRC_DIR}/shared/gui/GUI.h
)

# Tell the macro to link against the vAmp/FirstAmp API found in EXTLIB
BCI2000_INCLUDE( "VAMP" )
BCI2000_INCLUDE( "MATH" )
INCLUDE_DIRECTORIES( ${BCI2000_SRC_DIR}/shared/gui )

IF( WIN32 )
  # Create the signal source module
  BCI2000_ADD_SIGNAL_SOURCE_MODULE( 
    "${EXECUTABLE_NAME}" 
    "${SRC_PROJECT}" "${HDR_PROJECT}" 
  )
  ADD_CUSTOM_COMMAND(
    TARGET "${EXECUTABLE_NAME}"
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${BCI2000_SRC_DIR}/extlib/brainproducts/vamp/FirstAmp.dll" "${BCI2000_ROOT_DIR}/prog"
  )
ELSE()
  MESSAGE( "-- vAmp SDK does not provide libraries for this OS, will not build" )
ENDIF()
