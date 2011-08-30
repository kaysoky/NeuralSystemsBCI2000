###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for Neuroscan

# Set the executable name
SET( EXECUTABLE_NAME NeuroscanAccess )

# Set the project specific sources
SET( SRC_PROJECT
  Neuroscan.cpp
  NeuroscanADC.cpp 
#  NeuroscanThread.cpp
)
SET( HDR_PROJECT
  NeuroscanADC.h
  lib/NSDevice.h
#  NeuroscanThread.h
)

# Define the library
IF( BORLAND )
  LINK_DIRECTORIES( ${BCI2000_SRC_DIR}/contrib/SignalSource/NeuroscanAccessSDK/lib/omf )
ENDIF( BORLAND )

IF( MSVC )
  LINK_DIRECTORIES( ${BCI2000_SRC_DIR}/contrib/SignalSource/NeuroscanAccessSDK/lib/coff )
ENDIF( MSVC )

# Put everything together
IF( NOT MSVC AND NOT BORLAND )

  MESSAGE( "-- Neuroscan does not provide libraries for this compiler/OS, will not build" )

ELSE( NOT MSVC AND NOT BORLAND )

  # Create the signal source module
  BCI2000_ADD_SIGNAL_SOURCE_MODULE( 
    "${EXECUTABLE_NAME}" 
    "${SRC_PROJECT}" "${HDR_PROJECT}" 
  )

  # Link the Neuroscan Library
  TARGET_LINK_LIBRARIES( ${EXECUTABLE_NAME} NSDevice )

  ADD_CUSTOM_COMMAND(
    TARGET "${EXECUTABLE_NAME}"
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/lib/NSDevice.dll" "${BCI2000_ROOT_DIR}/prog"
  )

ENDIF( NOT MSVC AND NOT BORLAND )


