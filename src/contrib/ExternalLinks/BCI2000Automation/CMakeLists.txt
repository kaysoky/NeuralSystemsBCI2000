###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for BCI2000Automation

IF( WIN32 )

# Set the executable name
SET( EXECUTABLE_NAME BCI2000Automation )
IF( CMAKE_SIZEOF_VOID_P EQUAL 8 )
  SET( EXECUTABLE_NAME "${EXECUTABLE_NAME}64" )
ENDIF()

# DEBUG
MESSAGE( "-- Adding Project: ${EXECUTABLE_NAME}" )

# Set the project specific sources
SET( SRC_PROJECT
  BCI2000Automation.cpp
  BCI2000Automation_i.c
  BCI2000Automation.rc
  CBCI2000Remote.cpp
  Resource.h
  ${BCI2000_SRC_DIR}/shared/utils/SockStream.cpp
  ${BCI2000_SRC_DIR}/core/Operator/BCI2000Remote/BCI2000Remote.cpp  
  ${BCI2000_SRC_DIR}/core/Operator/BCI2000Remote/BCI2000Connection.cpp  
)

# Add COM library
BCI2000_INCLUDE( COM )

SET( LIBS
  ${LIBS}
  ws2_32
)

IF( NOT FAILED )
  BCI2000_ADD_TARGET( MODULE ${EXECUTABLE_NAME} ${SRC_PROJECT} )
  ADD_CUSTOM_COMMAND(
    TARGET "${EXECUTABLE_NAME}"
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/InstallAutomation.cmd" "${BCI2000_ROOT_DIR}/prog"
  )

ENDIF( NOT FAILED )

ENDIF( WIN32 )
