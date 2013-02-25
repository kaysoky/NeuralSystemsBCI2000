###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for the Emotiv module

# Set the executable name
SET( EXECUTABLE_NAME Emotiv )

# Set the project specific sources
SET( SRC_PROJECT
  EmotivADC.cpp 
  lib/edk.imports.cpp
)
SET( HDR_PROJECT
  EmotivADC.h
  lib/edk.imports.h
)

IF( DYNAMIC_IMPORTS )
ELSE()
  # Define the library
  LINK_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/lib/coff )
ENDIF()

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/lib/include )

BCI2000_INCLUDE( "SOURCEFILTER" )

IF( DYNAMIC_IMPORTS OR MSVC AND NOT CMAKE_CL_64 OR BORLAND )

# Create the signal source module
BCI2000_ADD_SIGNAL_SOURCE_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
)

IF( NOT DYNAMIC_IMPORTS )
  # Link the Emotiv Library
  TARGET_LINK_LIBRARIES( ${EXECUTABLE_NAME} edk )
ENDIF()

ADD_CUSTOM_COMMAND(
  TARGET "${EXECUTABLE_NAME}"
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/dll/edk.dll" "${BCI2000_ROOT_DIR}/prog"
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/dll/edk_utils.dll" "${BCI2000_ROOT_DIR}/prog"
)

ELSE()


  MESSAGE( "--- No Emotiv libraries for this Compiler/OS. Will not build." )
ENDIF()