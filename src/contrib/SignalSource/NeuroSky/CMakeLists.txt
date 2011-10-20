###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for NeuroSky

# Set the executable name
SET( EXECUTABLE_NAME NeuroSky )

# Set the project specific sources
SET( SRC_PROJECT
  NeuroSky.cpp
  NeuroSkyADC.cpp 
#  NeuroSkyThread.cpp
)
SET( HDR_PROJECT
  NeuroSkyADC.h
#  NeuroSkyThread.h
  lib/thinkgear.h
)

# Define the library
IF( BORLAND )
  LINK_DIRECTORIES( ${BCI2000_SRC_DIR}/contrib/SignalSource/NeuroSky/lib/omf )
#  MESSAGE( "-- Borland not yet supported for NeuroSky module.  Will not build" )
ENDIF( BORLAND )

IF( MSVC )
  LINK_DIRECTORIES( ${BCI2000_SRC_DIR}/contrib/SignalSource/NeuroSky/lib/coff )
ENDIF( MSVC )

# Put everything together
IF( NOT MSVC AND NOT BORLAND OR CMAKE_CL_64 )

  MESSAGE( "-- NeuroSky does not provide libraries for this compiler/OS, will not build" )

ELSE()

  # Create the signal source module
  BCI2000_ADD_SIGNAL_SOURCE_MODULE( 
    "${EXECUTABLE_NAME}" 
    "${SRC_PROJECT}" "${HDR_PROJECT}" 
  )

  # Link the NeuroSky Library
  TARGET_LINK_LIBRARIES( ${EXECUTABLE_NAME} thinkgear )
  
  ADD_CUSTOM_COMMAND(
    TARGET "${EXECUTABLE_NAME}"
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/lib/thinkgear.dll" "${BCI2000_ROOT_DIR}/prog"
  )

ENDIF()


