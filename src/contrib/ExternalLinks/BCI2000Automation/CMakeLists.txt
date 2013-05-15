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
)

SET( HDR_PROJECT
  BCI2000Automation.h
  CBCI2000Remote.h
  Resource.h
)

SET( SRC_FRAMEWORK 
  ${BCI2000_SRC_DIR}/shared/utils/SockStream.cpp
  ${BCI2000_SRC_DIR}/core/Operator/BCI2000Remote/BCI2000Remote.cpp  
  ${BCI2000_SRC_DIR}/core/Operator/BCI2000Remote/BCI2000Connection.cpp  
)

SET( HDR_FRAMEWORK 
  ${BCI2000_SRC_DIR}/shared/utils/SockStream.h
  ${BCI2000_SRC_DIR}/core/Operator/BCI2000Remote/BCI2000Remote.h  
  ${BCI2000_SRC_DIR}/core/Operator/BCI2000Remote/BCI2000Connection.h  
)

SOURCE_GROUP( Source\\BCI2000_Framework FILES ${SRC_FRAMEWORK} )
SET( SRC_BCI2000_FRAMEWORK
  ${SRC_BCI2000_FRAMEWORK}
  ${SRC_FRAMEWORK}
)

SOURCE_GROUP( Headers\\BCI2000_Framework FILES ${HDR_FRAMEWORK} )
SET( HDR_BCI2000_FRAMEWORK
  ${HRD_BCI2000_FRAMEWORK}
  ${HDR_FRAMEWORK}
)

INCLUDE_DIRECTORIES(
  ${BCI2000_SRC_DIR}/shared/utils
  ${BCI2000_SRC_DIR}/core/Operator/BCI2000Remote
)

# Set the Project Source Groups
SOURCE_GROUP( Source\\Project FILES ${SRC_PROJECT} )
SOURCE_GROUP( Headers\\Project FILES ${HDR_PROJECT} )

# Add COM library
BCI2000_INCLUDE( COM )

# Setup Extlib Dependencies
BCI2000_SETUP_EXTLIB_DEPENDENCIES( SRC_BCI2000_FRAMEWORK HDR_BCI2000_FRAMEWORK LIBS FAILED )
SET( LIBS
  ${LIBS}
  ws2_32
)

IF( NOT FAILED )
  # Add the executable to the project
  ADD_LIBRARY(
    ${EXECUTABLE_NAME} 
    MODULE
    ${SRC_BCI2000_FRAMEWORK} ${HDR_BCI2000_FRAMEWORK} 
    ${SRC_PROJECT} ${HDR_PROJECT}
  )
  TARGET_LINK_LIBRARIES( ${EXECUTABLE_NAME} ${LIBS} )
  SET_PROPERTY( TARGET "${EXECUTABLE_NAME}" PROPERTY PREFIX "" )
  # Set the project build folder
  SET_PROPERTY( TARGET ${EXECUTABLE_NAME} PROPERTY FOLDER "${DIR_NAME}" )

  ADD_CUSTOM_COMMAND(
    TARGET "${EXECUTABLE_NAME}"
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/InstallAutomation.cmd" "${BCI2000_ROOT_DIR}/prog"
  )

ENDIF( NOT FAILED )

ENDIF( WIN32 )
