###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for BioRadio

# Set the executable name
SET( EXECUTABLE_NAME BioRadio )

# Set the project specific sources
SET( SRC_PROJECT
  BioRadio.cpp
  BioRadioADC.cpp
  bioutils.cpp 
  BR150.cpp
)
SET( HDR_PROJECT
  BioRadioADC.h
  bioutils.h
  BR150.h
  BR_defines.h
  BioRadio150_SDK/BioRadio150DLL.h
)

LINK_DIRECTORIES( ${BCI2000_SRC_DIR}/contrib/SignalSource/BioRadio/BioRadio150_SDK )

IF( BORLAND OR MSVC )
  # Create the signal source module
  BCI2000_ADD_SIGNAL_SOURCE_MODULE( 
    "${EXECUTABLE_NAME}" 
    "${SRC_PROJECT}" "${HDR_PROJECT}" 
    "${INC_EXTLIB}"
  )
  ADD_CUSTOM_COMMAND(
  	TARGET "${EXECUTABLE_NAME}"
  	COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/BioRadio150_SDK/BioRadio150DLL.dll" "${BCI2000_ROOT_DIR}/prog"
  )
ELSE()
  MESSAGE( "-- BioRadio SDK does not provide libraries for this compiler/OS, will not build" )
ENDIF()

IF( BORLAND )
  TARGET_LINK_LIBRARIES( ${EXECUTABLE_NAME} BioRadio150DLL_Borland )
ELSEIF( MSVC )
  TARGET_LINK_LIBRARIES( ${EXECUTABLE_NAME} BioRadio150DLL_MSVC )
ENDIF()
