###########################################################################
## $Id$
## Authors: Paul Ignatenko <paul dot ignatenko at gmail dot com>
## Description: This module builds the libraries and sources needed to use Brain 
## Vision's actiCHamp amplifier.
##              
##              
###########################################################################

# Set the executable name
SET( EXECUTABLE_NAME actiCHampSource )

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

# Set the project specific sources
SET( SRC_PROJECT
  actiCHampBufferedADC.cpp
  actiCHampDevice.cpp
  ChannelRouter.cpp
  lib/ActiChamp.imports.cpp
)

SET( HDR_PROJECT
  actiCHampBufferedADC.h
  actiCHampDevice.h
  ChannelRouter.h
  lib/ActiChamp.imports.h
)

IF( USE_DYNAMIC_IMPORTS )
ELSE()
  LINK_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/lib/coff
  )
ENDIF()

BCI2000_ADD_SIGNAL_SOURCE_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
)

IF( NOT USE_DYNAMIC_IMPORTS )
  IF( CMAKE_SIZEOF_VOID_P EQUAL 8 )
      TARGET_LINK_LIBRARIES( ${EXECUTABLE_NAME} ActiChamp_x64.lib )
  ELSE()
      TARGET_LINK_LIBRARIES( ${EXECUTABLE_NAME} ActiChamp_x86.lib )
  ENDIF()
ENDIF()


ADD_CUSTOM_COMMAND(
  TARGET "${EXECUTABLE_NAME}"
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/lib/dylib/ActiChamp_x64.dll" "${BCI2000_ROOT_DIR}/prog"
)
ADD_CUSTOM_COMMAND(
  TARGET "${EXECUTABLE_NAME}"
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/lib/dylib/ActiChamp_x86.dll" "${BCI2000_ROOT_DIR}/prog"
)
ADD_CUSTOM_COMMAND(
  TARGET "${EXECUTABLE_NAME}"
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/lib/dylib/ActiChamp.bit" "${BCI2000_ROOT_DIR}/prog"
)
