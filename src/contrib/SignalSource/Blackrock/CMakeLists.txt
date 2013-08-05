###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for the Blackrock source module
###########################################################################

IF( CMAKE_SIZEOF_VOID_P EQUAL 8 OR NOT MSVC )
  MESSAGE( "Blackrock libraries not available for your current platform" )
ELSE()
# Set the executable name
SET( EXECUTABLE_NAME Blackrock )

# Set the project specific sources
SET( SRC_PROJECT
  BlackrockADC.cpp
  lib/cbsdk.imports.cpp
)

SET( HDR_PROJECT
  BlackrockADC.h
  lib/cbsdk.imports.h
)

INCLUDE_DIRECTORIES( lib )

# Add the source module to the Makefile or compiler project file:
BCI2000_ADD_SIGNAL_SOURCE_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
)

#TARGET_LINK_LIBRARIES( ${EXECUTABLE_NAME} cbsdk )

ADD_CUSTOM_COMMAND(
  TARGET "${EXECUTABLE_NAME}"
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/CursorTask_Blackrock.bat" "${BCI2000_ROOT_DIR}/batch"
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/P3Speller_Blackrock.bat" "${BCI2000_ROOT_DIR}/batch"
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/StimulusPresentation_Blackrock.bat" "${BCI2000_ROOT_DIR}/batch"
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/Blackrock.prm" "${BCI2000_ROOT_DIR}/parms/fragments/amplifiers"
)

IF( WIN32 )
  ADD_CUSTOM_COMMAND(
    TARGET "${EXECUTABLE_NAME}"
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/lib/dylib/cbsdk.dll" "${BCI2000_ROOT_DIR}/prog" 
  )
ELSE( WIN32 )
  ADD_CUSTOM_COMMAND(
    TARGET "${EXECUTABLE_NAME}"
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/lib/dylib/libcbsdk.so" "${BCI2000_ROOT_DIR}/prog" 
  )
ENDIF( WIN32 )

ENDIF() # CMAKE_SIZEOF_VOID_P EQUAL 8 OR NOT MSVC

