###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for Neuroscan

# Set the executable name
SET( EXECUTABLE_NAME NeuroscanAccess )

# Set the project specific sources
SET( SRC_PROJECT
  NeuroscanADC.cpp 
  lib/NSDevice.imports.cpp
)
SET( HDR_PROJECT
  NeuroscanADC.h
  lib/NSDevice.h
  lib/NSDevice.imports.h
)

IF( USE_DYNAMIC_IMPORTS )
ELSEIF( MSVC )
  LINK_DIRECTORIES( ${BCI2000_SRC_DIR}/contrib/SignalSource/NeuroscanAccessSDK/lib/coff )
ENDIF()

# Put everything together
IF( NOT USE_DYNAMIC_IMPORTS AND (NOT MSVC OR CMAKE_SIZEOF_VOID_P EQUAL 8) )

  MESSAGE( "-- Neuroscan does not provide libraries for this compiler/OS, will not build" )

ELSE()

  # Create the signal source module
  BCI2000_ADD_SIGNAL_SOURCE_MODULE( 
    "${EXECUTABLE_NAME}" 
    "${SRC_PROJECT}" "${HDR_PROJECT}" 
  )

  IF( NOT USE_DYNAMIC_IMPORTS )
    # Link the Neuroscan Library
    TARGET_LINK_LIBRARIES( ${EXECUTABLE_NAME} NSDevice )
  ENDIF()

  ADD_CUSTOM_COMMAND(
    TARGET "${EXECUTABLE_NAME}"
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/lib/NSDevice.dll" "${BCI2000_ROOT_DIR}/prog"
  )

ENDIF()


