///////////////////////////////////////////////////////////////////////////
// $Id$
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: C-style interface to the BCI2000 operator library.
//
// $BEGIN_BCI2000_LICENSE$
//
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2012: BCI2000 team and many external contributors ]
//
// BCI2000 is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// BCI2000 is distributed in the hope that it will be useful, but
//                         WITHOUT ANY WARRANTY
// - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.
//
// $END_BCI2000_LICENSE$
///////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include <sstream>
#include <cstring>
#include <set>
#include "BCI_OperatorLib.h"
#include "StateMachine.h"
#include "Lockable.h"
#include "ScriptInterpreter.h"
#include "Version.h"
#include "Param.h"
#include "TelnetServer.h"
#include "defines.h"

using namespace std;

StateMachine* gpStateMachine = NULL;
static TelnetServer* spTelnetServer = NULL;

typedef set<const char*> MemorySet;
static MemorySet sAllocatedMemory;

// An internal helper function that allocates output string buffers.
static const char* AllocateCopy( const char* inString )
{
  size_t len = ::strlen( inString );
  char* pCopy = new char[len + 1];
  if( pCopy != NULL )
  {
    ::strncpy( pCopy, inString, len + 1 );
    sAllocatedMemory.insert( pCopy );
  }
  return pCopy;
}

/*
function:  BCI_ReleaseObject
purpose:   Indicate that an object that has been allocated by the library is no longer
           needed by the library's client.
arguments: Object to be released, or NULL.
returns:   1 if successful, 0 otherwise.
*/
DLLEXPORT int
STDCALL BCI_ReleaseObject( const char* inObject )
{
  if( inObject == NULL )
    return 1;

  MemorySet::iterator i = sAllocatedMemory.find( inObject );
  if( i == sAllocatedMemory.end() )
    return 0;

  sAllocatedMemory.erase( i );
  delete[] inObject;
  return 1;
}

/*
function:  BCI_GetInfo
purpose:   Reports build and source version information.
arguments: None.
returns:   Pointer to a null-terminated string holding the information requested.
           The output buffer is allocated by the library, and should be released
           by the caller using BCI_ReleaseObject().
*/
DLLEXPORT const char*
STDCALL BCI_GetInfo( void )
{
  const char* versionInfo = BCI2000_VERSION;
  return AllocateCopy( versionInfo );
}

/*
function:  BCI_PutParameter
purpose:   Parses a BCI2000 parameter definition line, and adds the resulting
           parameter object to the internal parameter list, or changes the value of
           a parameter if it exists.
arguments: Pointer to a null-terminated parameter line string.
returns:   1 if successful, 0 otherwise.
*/
DLLEXPORT int
STDCALL BCI_PutParameter( const char* inParameterLine )
{
  if( gpStateMachine == NULL )
    return 0;

  Param param;
  istringstream iss( inParameterLine );
  if( !param.ReadFromStream( iss ) )
    return 0;

  {
    ::Lock<StateMachine> lock( *gpStateMachine );
    gpStateMachine->Parameters()[ param.Name() ] = param;
  }
  gpStateMachine->ParameterChange();
  gpStateMachine->ExecuteCallback( BCI_OnParameter, inParameterLine );
  return 1;
}

/*
function:  BCI_GetParameter
purpose:   Returns the parameter with the given index from the operator's internal
           parameter list.
arguments: Parameter index.
returns:   Pointer to a null-terminated string containing a parameter line, or NULL.
           The output buffer is allocated by the library, and should be released
           by the caller using BCI_ReleaseObject().
*/
DLLEXPORT const char*
STDCALL BCI_GetParameter( long inIndex )
{
  if( gpStateMachine == NULL )
    return NULL;

  const char* result = NULL;
  ::Lock<StateMachine> lock( *gpStateMachine );
  if( inIndex >= 0 && inIndex < gpStateMachine->Parameters().Size() )
  {
    ostringstream oss;
    oss << gpStateMachine->Parameters()[ inIndex ];
    result = AllocateCopy( oss.str().c_str() );
  }
  return result;
}

/*
function:  BCI_PutState
purpose:   Parses a BCI2000 state definition line, and adds the resulting
           state to the operator library's state list.
arguments: Pointer to a null-terminated state line string.
returns:   1 if successful, 0 otherwise.
*/
DLLEXPORT int
STDCALL BCI_PutState( const char* inStateLine )
{
  if( gpStateMachine == NULL )
    return 0;

  class State state;
  istringstream iss( inStateLine );
  if( !state.ReadFromStream( iss ) )
    return 0;
  {
    ::Lock<StateMachine> lock( *gpStateMachine );
    gpStateMachine->States().Add( state );
  }
  gpStateMachine->ExecuteCallback( BCI_OnState, inStateLine );
  return 1;
}

/*
function:  BCI_GetState
purpose:   Returns the state with the given index from the DLL's internal
           state list.
arguments: State index.
returns:   Pointer to a null-terminated string containing a state line, or NULL.
           The output buffer is allocated by the library, and should be released
           by the caller using BCI_ReleaseObject().
*/
DLLEXPORT const char*
STDCALL BCI_GetState( long inIndex )
{
  if( gpStateMachine == NULL )
    return NULL;

  const char* result = NULL;
  if( inIndex >= 0 && inIndex < gpStateMachine->States().Size() )
  {
    ostringstream oss;
    oss << gpStateMachine->States()[ inIndex ];
    result = AllocateCopy( oss.str().c_str() );
  }
  return result;
}

/*
function:  BCI_SetStateValue
purpose:   Sets the value of a state to a given value.
arguments: Pointer to a null-terminated state name string; new state value.
returns:   1 if successful, 0 otherwise.
*/
DLLEXPORT int
STDCALL BCI_SetStateValue( const char* inStateName, long inValue )
{
  if( gpStateMachine == NULL )
    return 0;

  return gpStateMachine->SetStateValue( inStateName, inValue );
}

/*
function:  BCI_GetStateValue
purpose:   Returns the value of a state.
arguments: Pointer to a null-terminated state name string.
returns:   State value, or 0 if the state does not exist.
*/
DLLEXPORT long
STDCALL BCI_GetStateValue( const char* inStateName )
{
  if( gpStateMachine == NULL )
    return 0;
  if( gpStateMachine->States().Exists( inStateName ) )
    return gpStateMachine->GetStateValue( inStateName );
  return 0;
}

/*
function:  BCI_PutEvent
purpose:   Parses a BCI2000 event definition line, and adds the resulting
           event to the operator library's event list.
arguments: Pointer to a null-terminated event line string.
returns:   1 if successful, 0 otherwise.
*/
DLLEXPORT int
STDCALL BCI_PutEvent( const char* inEventLine )
{
  if( gpStateMachine == NULL )
    return 0;

  class State event;
  istringstream iss( inEventLine );
  if( !event.ReadFromStream( iss ) )
    return 0;
  {
    ::Lock<StateMachine> lock( *gpStateMachine );
    gpStateMachine->Events().Add( event );
  }
  return 1;
}

/*
function:  BCI_GetEvent
purpose:   Returns the event with the given index from the DLL's internal
           event list.
arguments: Event index.
returns:   Pointer to a null-terminated string containing an event line, or NULL.
           The output buffer is allocated by the library, and should be released
           by the caller using BCI_ReleaseObject().
*/
DLLEXPORT const char*
STDCALL BCI_GetEvent( long inIndex )
{
  if( gpStateMachine == NULL )
    return NULL;

  const char* result = NULL;
  if( inIndex >= 0 && inIndex < gpStateMachine->Events().Size() )
  {
    ostringstream oss;
    oss << gpStateMachine->Events()[ inIndex ];
    result = AllocateCopy( oss.str().c_str() );
  }
  return result;
}

/*
function:  BCI_SetEvent
purpose:   Asynchronously sets an event to a given value.
arguments: Pointer to a null-terminated state name string; new event value.
returns:   1 if successful, 0 otherwise.
*/
DLLEXPORT int
STDCALL BCI_SetEvent( const char* inEventName, long inValue )
{
  if( gpStateMachine == NULL )
    return 0;
  return gpStateMachine->SetEvent( inEventName, inValue );
}

/*
function:  BCI_GetSignalChannels
purpose:   Returns the number of channels in the control signal.
arguments: None
returns:   Number of signal channels.
*/
DLLEXPORT int
STDCALL BCI_GetSignalChannels( void )
{
  if( !gpStateMachine )
    return 0;
  return gpStateMachine->ControlSignal().Channels();
}

/*
function:  BCI_GetSignalElements
purpose:   Returns the number of elements in the control signal.
arguments: None
returns:   Number of signal elements.
*/
DLLEXPORT int
STDCALL BCI_GetSignalElements( void )
{
  if( !gpStateMachine )
    return 0;
  return gpStateMachine->ControlSignal().Elements();
}

/*
function:  BCI_GetSignal
purpose:   Returns a value from the control signal.
arguments: Channel index, element index (zero-based)
returns:   Signal value, or 0 when indices out of bounds.
*/
DLLEXPORT float
STDCALL BCI_GetSignal( int inChannel, int inElement )
{
  if( !gpStateMachine )
    return 0;
  const GenericSignal& signal = gpStateMachine->ControlSignal();
  if( inChannel > 0 && inChannel < signal.Channels()
      && inElement > 0 && inElement < signal.Elements() )
    return static_cast<float>( signal( inChannel, inElement ) );
  return 0;
}

/*
function:  BCI_PutVisProperty
purpose:   Parses a BCI2000 vis property definition line, and adds the resulting
           property to the property list.
arguments: Pointer to a null-terminated vis ID string, numeric config ID, and
           a pointer to a null-terminated value string.
returns:   1 if successful, 0 otherwise.
*/
DLLEXPORT int
STDCALL BCI_PutVisProperty( const char* inVisID, int inCfgID, const char* inValue )
{
  if( !gpStateMachine )
    return 0;
  {
    ::Lock<StateMachine> lock( *gpStateMachine );
    gpStateMachine->Visualizations()[ inVisID ][ inCfgID ] = inValue;
  }
  gpStateMachine->ExecuteCallback( BCI_OnVisProperty, inVisID, inCfgID, inValue );
  return 1;
}

/*
function:  BCI_GetVisProperty
purpose:   Returns the property with the given index from the DLL's internal
           property list. When a visID is given, results are restricted to properties
           with the respective visID.
arguments: Pointer to a null-terminated vis ID string, numeric config ID.
returns:   Pointer to a null-terminated string containing a property line, or NULL.
           The output buffer is allocated by the library, and should be released
           by the caller using BCI_ReleaseObject().
*/
DLLEXPORT const char*
STDCALL BCI_GetVisProperty( const char* inVisID, int inCfgID )
{
  if( !gpStateMachine )
    return NULL;

  const char* result = NULL;
  ::Lock<StateMachine> lock( *gpStateMachine );
  VisTable::iterator i = gpStateMachine->Visualizations().find( inVisID );
  if( i != gpStateMachine->Visualizations().end() )
  {
    VisProperties::iterator j = i->second.find( inCfgID );
    if( j != i->second.end() )
      result = AllocateCopy( j->second.c_str() );
  }
  return result;
}

/*
function:  BCI_GetStateOfOperation
purpose:   Determines the externally visible state of the state machine, or
           the state of operation of the BCI2000 system.
arguments: None
returns:   State of operation.
*/
DLLEXPORT int
STDCALL BCI_GetStateOfOperation()
{
  if( gpStateMachine == NULL )
    return BCI_StateUnavailable;

  switch( gpStateMachine->SystemState() )
  {
    case StateMachine::Idle:
      return BCI_StateIdle;

    case StateMachine::WaitingForConnection:
    case StateMachine::Publishing:
      return BCI_StateStartup;

    case StateMachine::Information:
    case StateMachine::Initialization:
      return BCI_StateConnected;

    case StateMachine::Resting:
      return BCI_StateResting;

    case StateMachine::Suspended:
      return BCI_StateSuspended;

    case StateMachine::RestingParamsModified:
    case StateMachine::SuspendedParamsModified:
      return BCI_StateParamsModified;

    case StateMachine::Running:
      return BCI_StateRunning;

    case StateMachine::Transition:
    case StateMachine::SetConfigIssued:
    case StateMachine::RunningInitiated:
    case StateMachine::SuspendInitiated:
      return BCI_StateBusy;

    default:
      ;
  }
  return BCI_StateUnavailable;
}

/*
function:  BCI_GetConnectionInfo
purpose:   Obtains information about a core module connection.
argument:  Zero-based index of core module connection.
returns:   Pointer to a null-terminated string containing connection information.
           The output buffer is allocated by the library, and should be released
           by the caller using BCI_ReleaseObject().
*/
DLLEXPORT const char*
STDCALL BCI_GetConnectionInfo( int inIndex )
{
  if( gpStateMachine == NULL )
    return NULL;

  if( inIndex < 0 || inIndex >= gpStateMachine->NumConnections() )
    return NULL;

  struct StateMachine::ConnectionInfo info = gpStateMachine->Info( inIndex );
  ostringstream oss;
  oss << "Protocol Version: " << info.Version << "\n"
      << "Name: " << info.Name << "\n"
      << "Address: " << info.Address << "\n"
      << "Messages received: " << info.MessagesRecv << "\n"
      << "Messages sent: " << info.MessagesSent << "\n";
  return AllocateCopy( oss.str().c_str() );
}

/*
function:  BCI_GetCoreModuleStatus
purpose:   Obtains a core module's current status message.
argument:  Zero-based index of core module connection.
returns:   Pointer to a null-terminated string containing status information.
           The output buffer is allocated by the library, and should be released
           by the caller using BCI_ReleaseObject().
*/
DLLEXPORT const char*
STDCALL BCI_GetCoreModuleStatus( int inIndex )
{
  if( gpStateMachine == NULL )
    return NULL;

  if( inIndex < 0 || inIndex >= gpStateMachine->NumConnections() )
    return NULL;

  return AllocateCopy( gpStateMachine->Info( inIndex ).Status.c_str() );
}


/*
function:  BCI_Startup
purpose:   Startup of the operator controller object.
arguments: A string defining core module names and listening ports in the form
             <ip address> <name1>:<port1> <name2:port2> ... <nameN:portN>
           If NULL, a value of
             "Source:4000 SignalProcessing:4001 Application:4002"
           representing a standard BCI2000 configuration is used.
           The first argument specifies an IP address on which to listen,
           "localhost", or "134.2.131.251".
           In standard configuration, the Operator module listens on all available
           addresses.
returns:   1 if successful, 0 otherwise.
*/
DLLEXPORT int
STDCALL BCI_Startup( const char* inArguments )
{
  if( gpStateMachine == NULL )
    return 0;

  return gpStateMachine->Startup( inArguments );
}

/*
function:  BCI_Shutdown
purpose:   Close all connections opened by the library.
arguments: n/a
returns:   1 if successful, 0 otherwise.
*/
DLLEXPORT int
STDCALL BCI_Shutdown( void )
{
  if( gpStateMachine == NULL )
    return 0;

  gpStateMachine->Shutdown();
  return 1;
}

/*
function:  BCI_Initialize
purpose:   Initialize the library. Must be called before any other library
           function is used.
arguments: n/a
returns:   1 if no error occurred, 0 otherwise.
*/
DLLEXPORT int
STDCALL BCI_Initialize( void )
{
  if( gpStateMachine != NULL )
    return 0;

  gpStateMachine = new StateMachine;
  return 1;
}

/*
function:  BCI_TelnetListen
purpose:   Start a telnet server, listening at the given address.
arguments: Address in <ip>:<port> format, defaults to "localhost:3999".
returns:   1 if no error occurred, 0 otherwise.
*/
DLLEXPORT int
STDCALL BCI_TelnetListen( const char* inAddress )
{
  if( gpStateMachine == NULL )
    return 0;
  if( spTelnetServer != NULL )
    return 0;
  const char* pAddress = inAddress;
  if( !pAddress || !*pAddress )
    pAddress = "localhost:3999";
  spTelnetServer = new TelnetServer( *gpStateMachine, pAddress );
  return spTelnetServer != NULL;
}

/*
function:  BCI_TelnetClose
purpose:   Stop the telnet server, closing any open connections.
arguments: <n/a>
returns:   1 if no error occurred, 0 otherwise.
*/
DLLEXPORT int
STDCALL BCI_TelnetClose()
{
  delete spTelnetServer;
  spTelnetServer = NULL;
  return 1;
}

/*
function:  BCI_Dispose
purpose:   Dispose of resources allocated by the library.
arguments: n/a
returns:   1 if no error occurred, 0 otherwise.
*/
DLLEXPORT int
STDCALL BCI_Dispose( void )
{
  BCI_TelnetClose();
  delete gpStateMachine;
  gpStateMachine = NULL;
  return 1;
}

/*
function:  BCI_SetConfig
purpose:   Applies current parameter settings to the BCI2000 system.
arguments: n/a
returns:   1 if successful, 0 otherwise.
*/
DLLEXPORT int
STDCALL BCI_SetConfig( void )
{
  if( gpStateMachine == NULL )
    return 0;
  return gpStateMachine->SetConfig();
}

/*
function:  BCI_StartRun
purpose:   Starts a new run.
arguments: n/a
returns:   1 if successful, 0 otherwise.
*/
DLLEXPORT int
STDCALL BCI_StartRun( void )
{
  if( gpStateMachine == NULL )
    return 0;
  return gpStateMachine->StartRun();
}

/*
function:  BCI_StopRun
purpose:   Stops the current run.
arguments: n/a
returns:   1 if successful, 0 otherwise.
*/
DLLEXPORT int
STDCALL BCI_StopRun( void )
{
  if( gpStateMachine == NULL )
    return 0;
  return gpStateMachine->StopRun();
}

/*
function:  BCI_ExecuteScript
purpose:   Interprets and executes the specified script.
arguments: Null-terminated string specifying script commands.
returns:   0 if a syntax error is present, 1 otherwise.
*/
DLLEXPORT int
STDCALL BCI_ExecuteScript( const char* inScript )
{
  return gpStateMachine ? ScriptInterpreter( *gpStateMachine ).Execute( inScript ) : 0;
}

/*
function:  BCI_ExecuteScriptWithResult
purpose:   Interprets and executes the specified script.
arguments: Null-terminated string specifying script commands.
returns:   Pointer to a null-terminated string containing the result.
           In case of successful execution, the result of the last executed
           script command is returned. In case of a script error, the
           result string starts with a backslash (as an escape character),
           followed with "Error: ", and the actual error message.
           The result string is allocated by the library, and should be
           released by the caller using BCI_ReleaseObject().
*/
DLLEXPORT const char*
STDCALL BCI_ExecuteScriptWithResult( const char* inScript )
{
  const char* pResult = NULL;
  if( gpStateMachine )
  {
    ScriptInterpreter interpreter( *gpStateMachine );
    interpreter.Execute( inScript );
    pResult = AllocateCopy( interpreter.Result().c_str() );
  }
  else
  {
    pResult = AllocateCopy(
      "\\Error: Operator library not initialized when trying to execute a script"
    );
  }
  return pResult;
}

/*
function:  BCI_SetCallback
purpose:   Register a callback function. To clear a callback function,
           specify NULL as a function pointer.
arguments: Event ID, callback function pointer, data pointer.
returns:   1 if successful, 0 otherwise.
*/
DLLEXPORT int
STDCALL BCI_SetCallback( long inEventID, BCI_Function inFunction, void* inData )
{
  if( gpStateMachine == NULL )
    return 0;

  gpStateMachine->SetCallbackFunction( inEventID, inFunction );
  gpStateMachine->SetCallbackData( inEventID, inData );
  gpStateMachine->SetCallbackContext( inEventID, CallbackBase::CallingThread );
  return 1;
}

/*
function:  BCI_SetExternalCallback
purpose:   Register a callback function to be executed in an external thread on
           execution of BCI_CheckPendingCallback(). To clear a callback function,
           specify NULL as a function pointer.
arguments: Event ID, callback function pointer, data pointer.
returns:   1 if successful, 0 otherwise.
*/
DLLEXPORT int
STDCALL BCI_SetExternalCallback( long inEventID, BCI_Function inFunction, void* inData )
{
  int result = BCI_SetCallback( inEventID, inFunction, inData );
  if( result == 1 )
     gpStateMachine->SetCallbackContext( inEventID, CallbackBase::MainThread );
  return result;
}

/*
function:  BCI_GetCallbackFunction
purpose:   Get a registered callback function pointer.
arguments: Event ID.
returns:   Callback function pointer, or NULL if no callback function has been
           registered.
*/
DLLEXPORT BCI_Function
STDCALL BCI_GetCallbackFunction( long inEventID )
{
  if( gpStateMachine == NULL )
    return NULL;

  return gpStateMachine->CallbackFunction( inEventID );
}

/*
function:  BCI_GetCallbackData
purpose:   Get registered callback data. Callback data is the first argument
           to callback functions, and specified when calling
           SetCallback/SetExternalCallback.
arguments: Event ID.
returns:   Callback data, or NULL if no callback data has been
           registered.
*/
DLLEXPORT void*
STDCALL BCI_GetCallbackData( long inEventID )
{
  if( gpStateMachine == NULL )
    return NULL;

  return gpStateMachine->CallbackData( inEventID );
}

/*
function:  BCI_GetCallbackIsExternal
purpose:   Get information how callback was registered.
arguments: Event ID.
returns:   Returns 1 if the function was registered with BCI_SetExternalCallback(),
           and 0 if it was registered with BCI_SetCallback(), or when no callback
           was registered.
*/
DLLEXPORT int
STDCALL BCI_GetCallbackIsExternal( long inEventID )
{
  if( gpStateMachine == NULL )
    return 0;

  return gpStateMachine->CallbackContext( inEventID ) != CallbackBase::CallingThread;
}

/*
function:  BCI_CheckPendingCallback
purpose:   Call this function regularly from within an external thread you want
           external callbacks to run in.
arguments: None.
returns:   1 if successful, 0 otherwise.
*/
DLLEXPORT int
STDCALL BCI_CheckPendingCallback()
{
  int result = 0;
  if( gpStateMachine != NULL )
    result = gpStateMachine->CheckPendingCallback();
  return result;
}

