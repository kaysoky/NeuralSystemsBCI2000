###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for NeuroSky

# Set the executable name
SET( EXECUTABLE_NAME NeuroSky )

# Set the project specific sources
SET( SRC_PROJECT
  NeuroSkyADC.cpp 
  lib/thinkgear.imports.cpp
)
SET( HDR_PROJECT
  NeuroSkyADC.h
  lib/thinkgear.h
  lib/thinkgear.imports.h
)

IF( USE_DYNAMIC_IMPORTS )
ELSEIF( MSVC )
  LINK_DIRECTORIES( ${BCI2000_SRC_DIR}/contrib/SignalSource/NeuroSky/lib/coff )
ENDIF()

# Put everything together
IF( NOT USE_DYNAMIC_IMPORTS AND (NOT MSVC AND NOT BORLAND OR CMAKE_CL_64) )

  MESSAGE( "-- NeuroSky does not provide libraries for this compiler/OS, will not build" )

ELSE()

  # Create the signal source module
  BCI2000_ADD_SIGNAL_SOURCE_MODULE( 
    "${EXECUTABLE_NAME}" 
    "${SRC_PROJECT}" "${HDR_PROJECT}" 
  )

  IF( NOT USE_DYNAMIC_IMPORTS )
    # Link the NeuroSky Library
    TARGET_LINK_LIBRARIES( ${EXECUTABLE_NAME} thinkgear )
  ENDIF()
  
  ADD_CUSTOM_COMMAND(
    TARGET "${EXECUTABLE_NAME}"
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/lib/thinkgear.dll" "${BCI2000_ROOT_DIR}/prog"
  )

ENDIF()


