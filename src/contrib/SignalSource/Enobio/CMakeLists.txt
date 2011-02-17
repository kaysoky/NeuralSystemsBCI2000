###########################################################################
## $Id$
## Authors: masayo.haneda@starlab.es
## Description: Build information for ENOBIO

# Set the executable name
SET( EXECUTABLE_NAME ENOBIO )

# Set the project specific sources
SET( SRC_PROJECT
  ENOBIO.cpp
  ENOBIOADC.cpp 
)
SET( HDR_PROJECT
  ENOBIOADC.h
  lib/CEnobioCtrl.h
)

# Define the library
IF( BORLAND )
  #LINK_DIRECTORIES( ${BCI2000_SRC_DIR}/contrib/SignalSource/ENOBIO/lib/BORLAND )
  MESSAGE( "-- ENOBIO does not provide libraries for this compiler/OS,  Will not build" )

ENDIF( BORLAND )

IF( MSVC )
  LINK_DIRECTORIES( ${BCI2000_SRC_DIR}/contrib/SignalSource/ENOBIO/lib/MSVC )
ENDIF( MSVC )

# Put everything together
IF( NOT MSVC AND NOT BORLAND )
  MESSAGE( "-- ENOBIO does not provide libraries for this compiler/OS,  Will not build" )
ELSE( NOT MSVC AND NOT BORLAND )

  # Create the signal source module
  BCI2000_ADD_SIGNAL_SOURCE_MODULE( 
    "${EXECUTABLE_NAME}" 
    "${SRC_PROJECT}" "${HDR_PROJECT}" 
    "${INC_EXTLIB}"
  )

  # Link the Enobio Library
  TARGET_LINK_LIBRARIES( ${EXECUTABLE_NAME} EnobioDLL )

  ADD_CUSTOM_COMMAND(
  	TARGET "${EXECUTABLE_NAME}"
  	POST_BUILD
  	COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/lib/EnobioDLL.dll" "${BCI2000_ROOT_DIR}/prog"
  )

ENDIF( NOT MSVC AND NOT BORLAND )


