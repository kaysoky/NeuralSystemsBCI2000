###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for the OperatorLib

# Set the executable name
SET( NAME OperatorLib )

# DEBUG
MESSAGE( "-- Adding Project: " ${NAME} )

SET( SRC_PROJECT
  BCI_OperatorLib.cpp
  CallbackBase.cpp
  StateMachine.cpp
  TelnetServer.cpp
  ScriptInterpreter.cpp
  CommandInterpreter.cpp
)
SET( SRC_OBJECT_TYPES
  ObjectTypes/ObjectType.cpp
  ObjectTypes/ConditionType.cpp
  ObjectTypes/ImpliedType.cpp
  ObjectTypes/SystemTypes.cpp
  ObjectTypes/ParameterTypes.cpp
  ObjectTypes/StateTypes.cpp
  ObjectTypes/EventTypes.cpp
  ObjectTypes/VariableTypes.cpp
  ObjectTypes/SignalTypes.cpp
  ObjectTypes/VisPropertyTypes.cpp
  ObjectTypes/MessageTypes.cpp
  ObjectTypes/ScriptType.cpp
  ObjectTypes/ScriptEvents.cpp
  ObjectTypes/ExpressionType.cpp
  ObjectTypes/FileSystemTypes.cpp
  ObjectTypes/LineType.cpp
)
SET( SRC_SCRIPT_PARSER
  ScriptParser/Script.cpp
  ScriptParser/ParserToken.cpp
  ScriptParser/ParserNodes.cpp
  ScriptParser/ScriptParser.cpp
)

SET( HDR_PROJECT
  BCI_OperatorLib.h
  CallbackBase.h
  StateMachine.h
  VisTable.h
  TelnetServer.h
  ScriptInterpreter.h
  CommandInterpreter.h
)
SET( HDR_OBJECT_TYPES
  ObjectTypes/ObjectType.h
  ObjectTypes/ConditionType.h
  ObjectTypes/ImpliedType.h
  ObjectTypes/SystemTypes.h
  ObjectTypes/ParameterTypes.h
  ObjectTypes/StateTypes.h
  ObjectTypes/EventTypes.h
  ObjectTypes/VariableTypes.h
  ObjectTypes/SignalTypes.h
  ObjectTypes/VisPropertyTypes.h
  ObjectTypes/MessageTypes.h
  ObjectTypes/ScriptType.h
  ObjectTypes/ScriptEvents.h
  ObjectTypes/ExpressionType.h
  ObjectTypes/FileSystemTypes.h
  ObjectTypes/LineType.h
)
SET( HDR_SCRIPT_PARSER
  ScriptParser/Script.h
  ScriptParser/ParserToken.h
  ScriptParser/ParserNodes.h
  ScriptParser/ScriptParser.h
)

IF( WIN32 )
  SET( SRC_PROJECT
    ${SRC_PROJECT}
    Main.cpp
  )
ENDIF( WIN32 )

# Generate the required framework
INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/Core.cmake )

# Add missing framework files
SET( SRC_BCI2000_FRAMEWORK
  ${SRC_BCI2000_FRAMEWORK}
  ${BCI2000_SRC_DIR}/shared/bcistream/BCIError_opdll.cpp
)
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\bcistream FILES
  ${BCI2000_SRC_DIR}/shared/bcistream/BCIError_opdll.cpp )


# Set the Project Source Groups
SOURCE_GROUP( Source\\Project FILES ${SRC_PROJECT} )
SOURCE_GROUP( Headers\\Project FILES ${HDR_PROJECT} )
SOURCE_GROUP( Source\\Project\\ObjectTypes FILES ${SRC_OBJECT_TYPES} )
SOURCE_GROUP( Headers\\Project\\ObjectTypes FILES ${HDR_OBJECT_TYPES} )
SOURCE_GROUP( Source\\Project\\ScriptParser FILES ${SRC_SCRIPT_PARSER} )
SOURCE_GROUP( Headers\\Project\\ScriptParser FILES ${HDR_SCRIPT_PARSER} )

INCLUDE_DIRECTORIES( ObjectTypes ScriptParser )

# Add the executable to the project
ADD_LIBRARY(  
  ${NAME} 
  SHARED
  ${SRC_BCI2000_FRAMEWORK} ${HDR_BCI2000_FRAMEWORK} 
  ${SRC_PROJECT} ${HDR_PROJECT}
  ${SRC_OBJECT_TYPES} ${HDR_OBJECT_TYPES}
  ${SRC_SCRIPT_PARSER} ${HDR_SCRIPT_PARSER}
)

# Add Pre-processor defines
IF( WIN32 )
  SET_PROPERTY( TARGET ${NAME} APPEND PROPERTY COMPILE_FLAGS "-DLIBOPERATOR_LIBRARY" )
ENDIF( WIN32 )

TARGET_LINK_LIBRARIES( ${NAME} ${LIBS} )

# Set output name
SET_TARGET_PROPERTIES( ${NAME} PROPERTIES 
    PREFIX "" 
)

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

# Set the project build folder
SET_PROPERTY( TARGET ${NAME} PROPERTY FOLDER "${DIR_NAME}" )
