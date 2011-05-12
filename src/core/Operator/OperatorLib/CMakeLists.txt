###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for the OperatorLib

# Set the executable name
SET( NAME OperatorLib )

# DEBUG
MESSAGE( "-- Adding Project: " ${NAME} )

# Set the project specific sources
SET( SRC_PROJECT
  BCI_OperatorLib.cpp
  CallbackBase.cpp
  ScriptInterpreter.cpp
  StateMachine.cpp
)
SET( HDR_PROJECT
  BCI_OperatorLib.h
  CallbackBase.h
  ScriptInterpreter.h
  StateMachine.h
  VisTable.h
)

IF( WIN32 )
  SET( SRC_PROJECT
    ${SRC_PROJECT}
    Main.cpp
  )
ENDIF( WIN32 )

# Generate the required framework
INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/MinimalFramework.cmake )

# Add missing framework files
SET( SRC_BCI2000_FRAMEWORK
  ${SRC_BCI2000_FRAMEWORK}
  ${BCI2000_SRC_DIR}/shared/bcistream/BCIError_opdll.cpp
  ${BCI2000_SRC_DIR}/shared/utils/OSThread.cpp
  ${BCI2000_SRC_DIR}/shared/utils/OSMutex.cpp
  ${BCI2000_SRC_DIR}/shared/utils/OSEvent.cpp
)
SET( HDR_BCI2000_FRAMEWORK
  ${HDR_BCI2000_FRAMEWORK}
  ${BCI2000_SRC_DIR}/shared/utils/OSThread.h
  ${BCI2000_SRC_DIR}/shared/utils/OSMutex.h
  ${BCI2000_SRC_DIR}/shared/utils/OSEvent.h
)
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\bcistream FILES
  ${BCI2000_SRC_DIR}/shared/bcistream/BCIError_opdll.cpp )
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\utils FILES
  ${BCI2000_SRC_DIR}/shared/utils/OSThread.cpp
  ${BCI2000_SRC_DIR}/shared/utils/OSMutex.cpp
  ${BCI2000_SRC_DIR}/shared/utils/OSEvent.cpp 
)
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\utils FILES
  ${BCI2000_SRC_DIR}/shared/utils/OSThread.h
  ${BCI2000_SRC_DIR}/shared/utils/OSMutex.h
  ${BCI2000_SRC_DIR}/shared/utils/OSEvent.h
)


# Set the Project Source Groups
SOURCE_GROUP( Source\\Project FILES ${SRC_PROJECT} )
SOURCE_GROUP( Headers\\Project FILES ${HDR_PROJECT} )

# Add the executable to the project
ADD_LIBRARY(  
  ${NAME} 
  SHARED
  ${SRC_BCI2000_FRAMEWORK} ${HDR_BCI2000_FRAMEWORK} 
  ${SRC_PROJECT} ${HDR_PROJECT} 
)

# Add Pre-processor defines
IF( WIN32 )
  SET_PROPERTY( TARGET ${NAME} APPEND PROPERTY COMPILE_FLAGS "-DLIBOPERATOR_LIBRARY" )
ENDIF( WIN32 )

IF( WIN32 )
  TARGET_LINK_LIBRARIES( ${NAME} wsock32 )
ENDIF( WIN32 )

# Set output name
IF( WIN32 )
  SET_TARGET_PROPERTIES( ${NAME} PROPERTIES 
    PREFIX "" 
  )
ENDIF( WIN32 )

IF( MINGW )
  SET_TARGET_PROPERTIES( ${NAME} PROPERTIES 
    LINK_FLAGS -Wl,--add-stdcall-alias 
  )
ENDIF( MINGW )

# Fix library path for OSX
IF( APPLE )
  SET_TARGET_PROPERTIES( ${NAME} PROPERTIES
    LINK_FLAGS "-install_name @executable_path/OperatorLib.dylib"
  )
ENDIF( APPLE )
