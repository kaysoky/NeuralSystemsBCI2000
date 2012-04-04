###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for BCI2000PresentationLink

IF( WIN32 )

# Set the executable name
SET( EXECUTABLE_NAME BCI2000PresentationLink )

# DEBUG
MESSAGE( "-- Adding Project: ${EXECUTABLE_NAME}" )

# Set the project specific sources
SET( SRC_PROJECT
  BCI2000PresentationLink.cpp
  BCI2000DataPortExtension.cpp
  BCI2000DataPort.cpp
  BCI2000ActiveDataPort.cpp
  BCI2000PCLLibrary.cpp
  BCI2000ControllerType.cpp
  BCI2000Controller.cpp
  Utils.cpp
  BCI2000PresentationLink_i.c
  BCI2000PresentationLink.rc
)

SET( HDR_PROJECT
  Resource.h
  BCI2000DataPortExtension.h
  BCI2000DataPort.h
  BCI2000ActiveDataPort.h
  BCI2000PCLLibrary.h
  BCI2000ControllerType.h
  BCI2000Controller.h
  Utils.h
  BCI2000PresentationLink_i.h
)

SET( SRC_FRAMEWORK 
  ${BCI2000_SRC_DIR}/shared/bcistream/BCIException.cpp
  ${BCI2000_SRC_DIR}/shared/utils/VersionInfo.cpp
  ${BCI2000_SRC_DIR}/shared/utils/SockStream.cpp
  ${BCI2000_SRC_DIR}/core/Operator/BCI2000Remote/BCI2000Remote.cpp  
  ${BCI2000_SRC_DIR}/core/Operator/BCI2000Remote/BCI2000Connection.cpp  
)

SET( HDR_FRAMEWORK 
  ${BCI2000_SRC_DIR}/shared/config/Version.h
  ${BCI2000_SRC_DIR}/shared/bcistream/BCIException.h
  ${BCI2000_SRC_DIR}/shared/utils/VersionInfo.h
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
  ${BCI2000_SRC_DIR}/shared/bcistream
  ${BCI2000_SRC_DIR}/shared/utils
  ${BCI2000_SRC_DIR}/shared/config
  ${BCI2000_SRC_DIR}/core/Operator/BCI2000Remote
)

# Set the Project Source Groups
SOURCE_GROUP( Source\\Project FILES ${SRC_PROJECT} )
SOURCE_GROUP( Headers\\Project FILES ${HDR_PROJECT} )

# Add NBS headers and sources
SET( SRC_NBS
  ${BCI2000_SRC_DIR}/extlib/nbs/DataPortExtension/interfaces/DataPortExtension_i.c
  ${BCI2000_SRC_DIR}/extlib/nbs/PCLExtension/interface/PCLExtension_i.c
)
SOURCE_GROUP( Source\\NBS FILES ${SRC_NBS} )
SET( SRC_PROJECT
  ${SRC_PROJECT}
  ${SRC_NBS}
)

SET( HDR_NBS
  ${BCI2000_SRC_DIR}/extlib/nbs/DataPortExtension/interfaces/DataPortExtension.h
  ${BCI2000_SRC_DIR}/extlib/nbs/PCLExtension/interface/PCLExtension.h
)
SOURCE_GROUP( Headers\\NBS FILES ${HDR_NBS} )
INCLUDE_DIRECTORIES(
  ${BCI2000_SRC_DIR}/extlib/nbs/DataPortExtension/interfaces
  ${BCI2000_SRC_DIR}/extlib/nbs/PCLExtension/interface
)
SET( HDR_PROJECT
  ${HDR_PROJECT}
  ${HDR_NBS}
)

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

  ADD_CUSTOM_COMMAND(
    TARGET "${EXECUTABLE_NAME}"
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/InstallPresentationLink.cmd" "${BCI2000_ROOT_DIR}/prog"
  )

ENDIF( NOT FAILED )

ENDIF( WIN32 )
