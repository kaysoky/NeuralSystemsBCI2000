###########################################################################

## $Id: BAlertADC.cpp, 2011-12-12 v0.1 $

## Authors: Dr.Giby Raphael (graphael@b-alert.com)

## Description: Build information for the BAlert source module

###########################################################################


# Set the executable name

SET( EXECUTABLE_NAME BAlertSource )


# Set the project specific sources

SET( SRC_PROJECT

  BAlertADC.cpp

  lib/BAlert.imports.cpp

)


SET( HDR_PROJECT

  BAlertADC.h

  lib/BAlert.imports.h

)


IF( USE_DYNAMIC_IMPORTS )

ELSE()

  # Define the library

    LINK_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/lib/coff )

ENDIF()


INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/lib )


BCI2000_INCLUDE( "SOURCEFILTER" )


IF( USE_DYNAMIC_IMPORTS OR (MSVC AND NOT CMAKE_SIZEOF_VOID_P EQUAL 8) )


# Create the signal source module

BCI2000_ADD_SIGNAL_SOURCE_MODULE(
 
  "${EXECUTABLE_NAME}"
 
  "${SRC_PROJECT}" "${HDR_PROJECT}"

)


IF( NOT USE_DYNAMIC_IMPORTS )

  # link library

  TARGET_LINK_LIBRARIES( ${EXECUTABLE_NAME} BAlert )

ENDIF()


ADD_CUSTOM_COMMAND(

  TARGET "${EXECUTABLE_NAME}"

  POST_BUILD

  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/dll/BAlert.dll" "${BCI2000_ROOT_DIR}/prog"

  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/dll/SiUSBXp.dll" "${BCI2000_ROOT_DIR}/prog"

)


ELSE()



  MESSAGE( "--- No BAlert libraries for this Compiler/OS. Will not build." )

ENDIF()
