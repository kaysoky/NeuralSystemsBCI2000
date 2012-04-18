////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: juergen.mellinger@uni-tuebingen.de
// Description: A script interpreter type that handles global commands.
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
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "ImpliedType.h"
#include "SystemTypes.h"
#include "StateTypes.h"
#include "ParameterTypes.h"
#include "EventTypes.h"
#include "SignalTypes.h"
#include "MessageTypes.h"
#include "ScriptInterpreter.h"
#include "StateMachine.h"
#include "BCI_OperatorLib.h"
#include "BCIException.h"
#include "HybridString.h"

#if _WIN32
# include <windows.h>
#else // _WIN32
# include <cstdio>
#endif // _WIN32

using namespace std;
using namespace Interpreter;

static bool ExecuteSynchronously( const string&, ScriptInterpreter& );
static bool ExecuteAsynchronously( const string& executable, const string& arguments, ScriptInterpreter& );

ImpliedType ImpliedType::sInstance;
const ObjectType::MethodEntry ImpliedType::sMethodTable[] =
{
  METHOD( Get ), METHOD( Set ),
  METHOD( Wait ),
  METHOD( System ), METHOD( SetConfig ),
  METHOD( Start ), { "Resume", &Start }, METHOD( Stop ), { "Suspend", &Stop },
  METHOD( Startup ), METHOD( Shutdown ), METHOD( Reset ),
  METHOD( Quit ), { "Exit", &Quit },
  METHOD( Version ),
  METHOD( Log ), METHOD( Warn ), METHOD( Error ),
  END
};

void
ImpliedType::OnHelp( ScriptInterpreter& inInterpreter ) const
{
  inInterpreter.Out() << "Global commands: ";
  this->ListMethods( inInterpreter );

  const ObjectType* pType = ObjectType::Next( this );
  if( pType && pType != this )
    inInterpreter.Out() << "\nCommands for objects of type";
  while( pType && pType != this )
  {
    inInterpreter.Out() << "\n " << pType->Name() << ": ";
    pType->ListMethods( inInterpreter );
    pType = ObjectType::Next( pType );
  }
  const char* pHelpString = NULL;
  inInterpreter.StateMachine().ExecuteCallback( BCI_OnScriptHelp, &pHelpString );
  if( pHelpString )
    inInterpreter.Out() << "\nApplication-defined commands: " << pHelpString;
}

bool
ImpliedType::Get( ScriptInterpreter& inInterpreter )
{
  string object = inInterpreter.GetToken();
  inInterpreter.Unget();
  ScriptInterpreter::ArgumentList args;
  inInterpreter.ParseArguments( object, args );
  if( !::stricmp( object.c_str(), "Signal" ) )
    return Interpreter::SignalType::Get( inInterpreter );
  if( inInterpreter.StateMachine().States().Exists( object ) )
    return StateType::Get( inInterpreter );
  if( inInterpreter.StateMachine().Parameters().Exists( object ) )
    return ParameterType::Get( inInterpreter );
  return false;
}

bool
ImpliedType::Set( ScriptInterpreter& inInterpreter )
{
  string object = inInterpreter.GetToken();
  inInterpreter.Unget();
  ScriptInterpreter::ArgumentList args;
  inInterpreter.ParseArguments( object, args );
  if( inInterpreter.StateMachine().Events().Exists( object ) )
    return EventType::Set( inInterpreter );
  if( inInterpreter.StateMachine().States().Exists( object ) )
    return StateType::Set( inInterpreter );
  if( inInterpreter.StateMachine().Parameters().Exists( object ) )
    return ParameterType::Set( inInterpreter );
  return false;
}

bool
ImpliedType::Wait( ScriptInterpreter& inInterpreter )
{
  string token = inInterpreter.GetToken();
  if( !::stricmp( token.c_str(), "for" ) )
    return Interpreter::SystemType::WaitFor( inInterpreter );
  return false;
}

bool
ImpliedType::SetConfig( ScriptInterpreter& inInterpreter )
{
  return SystemType::SetConfig( inInterpreter );
}

bool
ImpliedType::Start( ScriptInterpreter& inInterpreter )
{
  return SystemType::Start( inInterpreter );
}

bool
ImpliedType::Stop( ScriptInterpreter& inInterpreter )
{
  return SystemType::Stop( inInterpreter );
}

bool
ImpliedType::Startup( ScriptInterpreter& inInterpreter )
{
  return SystemType::Startup( inInterpreter );
}

bool
ImpliedType::Shutdown( ScriptInterpreter& inInterpreter )
{
  return SystemType::Shutdown( inInterpreter );
}

bool
ImpliedType::Reset( ScriptInterpreter& inInterpreter )
{
  return SystemType::Reset( inInterpreter );
}

bool
ImpliedType::Quit( ScriptInterpreter& inInterpreter )
{
  return SystemType::Quit( inInterpreter );
}

bool
ImpliedType::Version( ScriptInterpreter& inInterpreter )
{
  return SystemType::GetVersion( inInterpreter );
}

bool
ImpliedType::Log( ScriptInterpreter& inInterpreter )
{
  return MessageType::Log( inInterpreter );
}

bool
ImpliedType::Warn( ScriptInterpreter& inInterpreter )
{
  return WarningType::Issue( inInterpreter );
}

bool
ImpliedType::Error( ScriptInterpreter& inInterpreter )
{
  return ErrorType::Report( inInterpreter );
}

bool
ImpliedType::System( ScriptInterpreter& inInterpreter )
{
#if _WIN32
  string command = "cmd /c " + inInterpreter.GetRemainder();
#else
  string command = "/bin/sh -c " + inInterpreter.GetRemainder();
#endif
  return ExecuteSynchronously( command, inInterpreter );
}


//// ExecutableType
ExecutableType ExecutableType::sInstance;
const ObjectType::MethodEntry ExecutableType::sMethodTable[] =
{
  METHOD( Start ), { "Run", &Start },
  END
};

bool
ExecutableType::Start( ScriptInterpreter& inInterpreter )
{
  string executable = inInterpreter.GetToken(),
         arguments = inInterpreter.GetRemainder();
  return ExecuteAsynchronously( executable, arguments, inInterpreter );
}

bool
ExecuteSynchronously( const string& inCommand, ScriptInterpreter& inInterpreter )
{
  bool success = false;
  int exitCode = 0;
  static const int bufferSize = 512;
  char buffer[bufferSize];

#if _WIN32
  // In Windows, popen() does not work unless a console exists in the application.
  // Thus, we need to use CreatePipe() in conjunction with CreateProcess().
  // The remaining code ensures that pipe handles are set up to be closed automatically
  // when the created process terminates. This is important because ReadFile() would
  // hang otherwise.
  // MS KB Article ID 190351: How to spawn console processes with redirected standard handles.
  BOOL result = TRUE;
  SECURITY_ATTRIBUTES sa;
  sa.nLength = sizeof( sa );
  sa.bInheritHandle = TRUE;
  sa.lpSecurityDescriptor = NULL;
  HANDLE pipeReadTmp = NULL,
         pipeRead = NULL,
         pipeWrite = NULL,
         pipeWrite2 = NULL;
  result &= ::CreatePipe( &pipeReadTmp, &pipeWrite, &sa, 0 );
  result &= ::DuplicateHandle( ::GetCurrentProcess(), pipeWrite, ::GetCurrentProcess(), &pipeWrite2, 0, TRUE, DUPLICATE_SAME_ACCESS );
  result &= ::DuplicateHandle( ::GetCurrentProcess(), pipeReadTmp, ::GetCurrentProcess(), &pipeRead, 0, FALSE, DUPLICATE_SAME_ACCESS );
  result &= ::CloseHandle( pipeReadTmp );

  PROCESS_INFORMATION procInfo;
  ::ZeroMemory( &procInfo, sizeof( procInfo ) );
  STARTUPINFO startInfo;
  ::ZeroMemory( &startInfo, sizeof( startInfo ) );
  startInfo.cb = sizeof( startInfo );
  startInfo.hStdError = pipeWrite2;
  startInfo.hStdOutput = pipeWrite;
  startInfo.hStdInput = ::GetStdHandle( STD_INPUT_HANDLE );
  startInfo.wShowWindow = SW_SHOWNA;
  startInfo.dwFlags |= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;

  char* pCommand = new char[inCommand.length() + 1];
  ::strcpy( pCommand, inCommand.c_str() );
  result &= ::CreateProcessA( NULL, pCommand, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &startInfo, &procInfo );
  delete[] pCommand;
  result &= ::CloseHandle( pipeWrite );
  result &= ::CloseHandle( pipeWrite2 );

  DWORD dwExitCode;
  while( ( result &= ::GetExitCodeProcess( procInfo.hProcess, &dwExitCode ) ) && dwExitCode == STILL_ACTIVE )
  {
    DWORD bytesRead;
    while( ::ReadFile( pipeRead, buffer, bufferSize, &bytesRead, NULL ) && bytesRead > 0 )
      inInterpreter.Out() << string( buffer, bytesRead );
  }
  exitCode = dwExitCode;
  ::CloseHandle( pipeRead );
  ::CloseHandle( procInfo.hProcess );
  ::CloseHandle( procInfo.hThread );
  success = ( result == TRUE );

#else // _WIN32

  FILE* pipe = ::popen( inCommand.c_str(), "rt" );
  if( pipe != NULL )
  {
    while( ::fgets( buffer, bufferSize, pipe ) )
      inInterpreter.Out() << buffer;
    exitCode = ::pclose( pipe );
    success = true;
  }

#endif // _WIN32

  if( !success )
    throw bciexception_( "Could not run \"" << inCommand.c_str() << "\"" );
  if( exitCode != 0 )
    inInterpreter.Out() << "\\ExitCode: " << exitCode;
  inInterpreter.Log() << "Executed \"" << inCommand.c_str() << "\"";
  return true;
}

bool
ExecuteAsynchronously( const string& inExecutable, const string& inArguments, ScriptInterpreter& inInterpreter )
{
  bool success = false;
  int exitCode = 0;

#if _WIN32

  string executable = inExecutable,
         extension = ".exe";
  if( executable.length() < extension.length()
    || ::stricmp( extension.c_str(), executable.substr( executable.length() - extension.length() ).c_str() ) )
    executable += extension;

  // When CreateProcess() is used to start up a core module, listening sockets in the Operator module are not closed properly.
  // ShellExecute() seems not to have this problem.
  SHELLEXECUTEINFOA info = { 0 };
  info.cbSize = sizeof( info );
  info.fMask = SEE_MASK_NOASYNC | SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI;
  info.hwnd = NULL;
  info.lpVerb = "open";
  info.lpFile = executable.c_str();
  info.lpParameters = inArguments.c_str();
  info.nShow = SW_SHOWNA;
  success = ::ShellExecuteExA( &info );
  if( success )
  {
    DWORD dwExitCode = 0;
    ::WaitForInputIdle( info.hProcess, INFINITE );
    ::GetExitCodeProcess( info.hProcess, &dwExitCode );
    if( STILL_ACTIVE != dwExitCode )
      exitCode = dwExitCode;
    ::CloseHandle( info.hProcess );
  }

#else // _WIN32

  exitCode = ::system( ( inCommand + " &" ).c_str() );
  success = ( 0 == exitCode );

#endif // _WIN32

  if( !success )
    throw bciexception_( "Could not run \"" << executable.c_str() << "\"" );
  if( exitCode != 0 )
    inInterpreter.Out() << "\\ExitCode: " << exitCode;
  inInterpreter.Log() << "Executed \"" << executable.c_str() << "\"";
  return true;
}
