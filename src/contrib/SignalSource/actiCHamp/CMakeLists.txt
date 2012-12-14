###########################################################################
## $Id:  $
## Authors: Paul Ignatenko <paul dot ignatenko at gmail dot com>
## Description: This module builds the libraries and sources needed to use Brain 
## Vision's actiCHamp amplifier.
##              
##              
###########################################################################

# Set the executable name
SET( EXECUTABLE_NAME actiCHampSource )

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}/externals/include
)

LINK_DIRECTORIES(
  ${BCI2000_ROOT_DIR}/prog,
  ${CMAKE_CURRENT_SOURCE_DIR}/externals/lib/coff
)

ADD_CUSTOM_COMMAND(
  TARGET "actiCHampSource"
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/externals/dylib/ActiChamp_x64.dll" "${BCI2000_ROOT_DIR}/prog"
)
ADD_CUSTOM_COMMAND(
  TARGET "actiCHampSource"
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/externals/dylib/ActiChamp_x86.dll" "${BCI2000_ROOT_DIR}/prog"
)
ADD_CUSTOM_COMMAND(
  TARGET "actiCHampSource"
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/externals/dylib/ActiChamp.bit" "${BCI2000_ROOT_DIR}/prog"
)

# Set the project specific sources
SET( SRC_PROJECT
  actiCHampBufferedADC.cpp
  actiCHampDevice.cpp
)

SET( HDR_PROJECT
  actiCHampBufferedADC.h
  actiCHampDevice.h
)

BCI2000_ADD_SIGNAL_SOURCE_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
)

IF( CMAKE_CL_64 )
    TARGET_LINK_LIBRARIES( ${EXECUTABLE_NAME} ActiChamp_x64.lib 
        actiCHampDevice.lib )
ELSE( CMAKE_CL_64 )
    TARGET_LINK_LIBRARIES( ${EXECUTABLE_NAME} ActiChamp_x86.lib 
        actiCHampDevice.lib )
ENDIF( CMAKE_CL_64 )

