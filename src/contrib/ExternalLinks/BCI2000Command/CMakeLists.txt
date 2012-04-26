###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for BCI2000Command

# Set the executable name
SET( EXECUTABLE_NAME BCI2000Command )

# DEBUG
MESSAGE( "-- Adding Project: ${EXECUTABLE_NAME}" )

ADD_DEFINITIONS(
  -DEXECUTABLE_NAME=\"${EXECUTABLE_NAME}\"
  -DOPERATOR_NAME=\"Operator\"
)

# Set the project specific sources
SET( SRC_PROJECT
  BCI2000Command.cpp
)

SET( HDR_PROJECT
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

# Setup Extlib Dependencies
BCI2000_SETUP_EXTLIB_DEPENDENCIES( SRC_BCI2000_FRAMEWORK HDR_BCI2000_FRAMEWORK LIBS FAILED )
SET( LIBS
  ${LIBS}
  ws2_32
  shlwapi
)

IF( NOT FAILED )
  # Add the executable to the project
  ADD_EXECUTABLE(
    ${EXECUTABLE_NAME} 
    ${SRC_BCI2000_FRAMEWORK} ${HDR_BCI2000_FRAMEWORK} 
    ${SRC_PROJECT} ${HDR_PROJECT}
  )
  TARGET_LINK_LIBRARIES( ${EXECUTABLE_NAME} ${LIBS} )

ENDIF( NOT FAILED )
