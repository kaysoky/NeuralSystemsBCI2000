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

  ObjectTypes/ObjectType.cpp
  ObjectTypes/ConditionType.cpp
  ObjectTypes/ImpliedType.cpp
  ObjectTypes/SystemTypes.cpp
  ObjectTypes/SystemStates.cpp
  ObjectTypes/ParameterTypes.cpp
  ObjectTypes/StateTypes.cpp
  ObjectTypes/EventTypes.cpp
  ObjectTypes/VariableTypes.cpp
  ObjectTypes/SignalTypes.cpp
  ObjectTypes/VisPropertyTypes.cpp
  ObjectTypes/MessageTypes.cpp
  ObjectTypes/ScriptType.cpp
  ObjectTypes/ScriptEvents.cpp
  ObjectTypes/InterpreterExpression.cpp
  ObjectTypes/Watches.cpp
  ObjectTypes/WatchTypes.cpp
  ObjectTypes/ExpressionType.cpp
  ObjectTypes/FileSystemTypes.cpp
  ObjectTypes/LineType.cpp

  ScriptParser/Script.cpp
  ScriptParser/ParserToken.cpp
  ScriptParser/ParserNodes.cpp
  ScriptParser/ScriptParser.cpp

  ${BCI2000_SRC_DIR}/shared/bcistream/BCIStream_opdll.cpp
)

IF( WIN32 )
  SET( SRC_PROJECT
    ${SRC_PROJECT}
    Main.cpp
  )
ENDIF( WIN32 )

INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/Core.cmake )

# Add the executable to the project
BCI2000_ADD_TARGET( DLL ${NAME} ${SRC_PROJECT} )

# Add Pre-processor defines
IF( WIN32 )
  SET_PROPERTY( TARGET ${NAME} APPEND PROPERTY COMPILE_FLAGS "-DLIBOPERATOR_LIBRARY" )
ENDIF( WIN32 )
