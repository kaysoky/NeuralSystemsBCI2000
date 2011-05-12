///////////////////////////////////////////////////////////////////////////
// $Id$
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: C-style interface to the BCI2000 operator library.
//
// $BEGIN_BCI2000_LICENSE$
// 
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2011: BCI2000 team and many external contributors ]
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
#include "ScriptInterpreter.h"
#include "Version.h"
#include "Param.h"
#include "defines.h"

using namespace std;

StateMachine* gpStateMachine = NULL;
static ScriptInterpreter* spInterpreter = NULL;

typedef set<const char*> MemorySet;
static MemorySet sAllocatedMemory;

// An internal helper function that allocates output string buffers.
static const char* AllocateCopy( const char* inString )
{
  int len = ::strlen( inString );
  char* pCopy = new char[ len + 1 ];
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

  gpStateMachine->LockData();
  gpStateMachine->Parameters()[ param.Name() ] = param;
  gpStateMachine->UnlockData();
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
  gpStateMachine->LockData();
  if( inIndex >= 0 && inIndex < gpStateMachine->Parameters().Size() )
  {
    ostringstream oss;
    oss << gpStateMachine->Parameters()[ inIndex ];
    result = AllocateCopy( oss.str().c_str() );
  }
  gpStateMachine->UnlockData();
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
  gpStateMachine->LockData();
  gpStateMachine->States().Add( state );
  gpStateMachine->UnlockData();
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
  gpStateMachine->LockData();
  gpStateMachine->Visualizations()[ inVisID ][ inCfgID ] = inValue;
  gpStateMachine->UnlockData();
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
  gpStateMachine->LockData();
  VisTable::iterator i = gpStateMachine->Visualizations().find( inVisID );
  if( i != gpStateMachine->Visualizations().end() )
  {
    VisProperties::iterator j = i->second.find( inCfgID );
    if( j != i->second.end() )
      result = AllocateCopy( j->second.c_str() );
  }
  gpStateMachine->UnlockData();
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
    case StateMachine::Publishing:
      return BCI_StateStartup;

    case StateMachine::Information:
    case StateMachine::Initialization:
      return BCI_StateInitialization;

    case StateMachine::Resting:
      return BCI_StateResting;

    case StateMachine::Suspended:
      return BCI_StateSuspended;

    case StateMachine::RestingParamsModified:
    case StateMachine::SuspendedParamsModified:
    return BCI_StateParamsModified;

  case StateMachine::Running:
      return BCI_StateRunning;

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
      << "Messages sent: " << info.MessagesSent << "\n"
      << "Parameters received: " << info.ParametersRecv << "\n"
      << "Parameters sent: " << info.ParametersSent << "\n"
      << "States received: " << info.StatesRecv << "\n"
      << "States sent: " << info.StatesSent << "\n"
      << "State vectors received: " << info.StateVecsRecv << "\n"
      << "State vectors sent: " << info.StateVecsSent << "\n"
      << "Data received: " << info.DataRecv << "\n";
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
arguments: A string defining core module listening addresses in the form
             <ip1>:<port1> <ip2:port2> ... <ipN:portN>
           If NULL, a value of
             localhost:4000 localhost:4001 localhost:4002
           reflecting the standard BCI2000 configuration is used.
returns:   1 if successful, 0 otherwise.
*/
DLLEXPORT int
STDCALL BCI_Startup( const char* inModuleList )
{
  if( gpStateMachine == NULL )
    return 0;

  return gpStateMachine->Startup( inModuleList );
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
  spInterpreter = new ScriptInterpreter( *gpStateMachine );
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
  delete spInterpreter;
  spInterpreter = NULL;
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
  if( spInterpreter == NULL )
    return 0;
  return spInterpreter->Execute( inScript );
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

