###########################################################################
## $Id: BAlertADC.cpp, 2011-12-12 v0.1 $
## Authors: Dr.Giby Raphael (graphael@b-alert.com)
## Description: Build information for the BAlert source module
###########################################################################

# Set the executable name
SET( EXECUTABLE_NAME BAlertSource )

# Set the project specific sources
SET( SRC_PROJECT
  BAlert.cpp           
  BAlertADC.cpp
)

SET( HDR_PROJECT
  BAlertADC.h
)

# Define the library
IF( BORLAND )
  LINK_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/lib/omf )
ELSE( BORLAND )
  LINK_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/lib/coff )
ENDIF( BORLAND )

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/lib/include )

BCI2000_INCLUDE( "SOURCEFILTER" )

IF( MSVC AND NOT CMAKE_CL_64 OR BORLAND )

# Create the signal source module
BCI2000_ADD_SIGNAL_SOURCE_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
)

# link library
TARGET_LINK_LIBRARIES( ${EXECUTABLE_NAME} BAlert )

ADD_CUSTOM_COMMAND(
  TARGET "${EXECUTABLE_NAME}"
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/dll/BAlert.dll" "${BCI2000_ROOT_DIR}/prog"
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/dll/SiUSBXp.dll" "${BCI2000_ROOT_DIR}/prog"
)

ELSE()


  MESSAGE( "--- No BAlert libraries for this Compiler/OS. Will not build." )
ENDIF()
