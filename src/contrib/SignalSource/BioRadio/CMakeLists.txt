###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for BioRadio

# Set the executable name
SET( EXECUTABLE_NAME BioRadio )

# Set the project specific sources
SET( SRC_PROJECT
  BioRadioADC.cpp
  bioutils.cpp 
  BR150.cpp
  BioRadio150_SDK/BioRadio150DLL.imports.cpp
)
SET( HDR_PROJECT
  BioRadioADC.h
  bioutils.h
  BR150.h
  BR_defines.h
  BioRadio150_SDK/BioRadio150DLL.h
  BioRadio150_SDK/BioRadio150DLL.imports.h
)

IF( USE_DYNAMIC_IMPORTS )
ELSE()
  LINK_DIRECTORIES( ${BCI2000_SRC_DIR}/contrib/SignalSource/BioRadio/BioRadio150_SDK )
ENDIF()

IF( USE_DYNAMIC_IMPORTS OR BORLAND OR MSVC AND NOT CMAKE_CL_64 )
  # Create the signal source module
  BCI2000_ADD_SIGNAL_SOURCE_MODULE( 
    "${EXECUTABLE_NAME}" 
    "${SRC_PROJECT}" "${HDR_PROJECT}" 
  )
  ADD_CUSTOM_COMMAND(
    TARGET "${EXECUTABLE_NAME}"
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/BioRadio150_SDK/BioRadio150DLL.dll" "${BCI2000_ROOT_DIR}/prog"
  )
ELSE()
  MESSAGE( "-- BioRadio SDK does not provide libraries for this compiler/OS, will not build" )
ENDIF()

IF( NOT USE_DYNAMIC_IMPORTS )
  IF( BORLAND )
    TARGET_LINK_LIBRARIES( ${EXECUTABLE_NAME} BioRadio150DLL_Borland )
  ELSEIF( MSVC AND NOT CMAKE_CL_64 )
    TARGET_LINK_LIBRARIES( ${EXECUTABLE_NAME} BioRadio150DLL_MSVC )
  ENDIF()
ENDIF()
