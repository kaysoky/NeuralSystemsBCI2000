//////////////////////////////////////////////////////////////////////
// $Id$
// Authors: juergen.mellinger@uni-tuebingen.de
// Description: A wrapper class representing a telnet connection to
//   BCI2000.
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
///////////////////////////////////////////////////////////////////////
#include "BCI2000Connection.h"

#include <sstream>
#include <cstdlib>

#if _WIN32
#include <Shlwapi.h>
#endif // _WIN32

using namespace std;

BCI2000Connection&
BCI2000Connection::WindowVisible( bool inVisible )
{
  mWindowVisible = inVisible;
  if( mSocket.connected() )
  {
    if( mWindowVisible )
      Execute( "show window" );
    else
      Execute( "hide window" );
  }
  return *this;
}

BCI2000Connection&
BCI2000Connection::WindowTitle( const std::string& inTitle )
{
  mWindowTitle = inTitle;
  if( mSocket.connected() )
    Execute( "set title \"" + inTitle + "\"" );
  return *this;
}

bool
BCI2000Connection::Connect()
{
  Disconnect();
  mResult.clear();
  bool success = true;
  if( mTelnetAddress.empty() )
    mTelnetAddress = DefaultTelnetAddress();
  if( !mOperatorPath.empty() )
  {
    success = Run( mOperatorPath );
    mTerminateOperator = success;
  }
  if( success )
  {
    mSocket.open( mTelnetAddress.c_str() );
    mConnection.open( mSocket );
    success = mConnection.is_open();
    if( success )
      success = mSocket.wait_for_read( static_cast<int>( 1e3 * mTimeout ) );
    if( !success )
      mResult = "Could not connect to Operator module at " + mTelnetAddress;
    else while( mConnection.rdbuf()->in_avail() )
      mConnection.ignore();
  }
  if( success )
  {
    WindowVisible( mWindowVisible );
    WindowTitle( mWindowTitle );
  }
  return success;
}

bool
BCI2000Connection::Connect( const BCI2000Connection& inRemote )
{
  mWindowVisible = inRemote.mWindowVisible;
  mWindowTitle = inRemote.mWindowTitle;

  mResult.clear();
  bool success = inRemote.mSocket.is_open();
  if( success )
    success = this->OperatorPath( "" )
                   .TelnetAddress( inRemote.TelnetAddress() )
                   .Connect();
  return success;
}

bool
BCI2000Connection::Disconnect()
{
  mResult.clear();
  if( mSocket.connected() && mTerminateOperator )
    Quit();
  mTerminateOperator = false;
  mSocket.close();
  mConnection.close();
  mConnection.clear();
  return true;
}

int
BCI2000Connection::Execute( const string& inCommand )
{
  mResult.clear();
  if( !mSocket.connected() )
  {
    mResult = "Not connected (call BCI2000Connection::Connect() to establish a connection)";
    return -1;
  }
  mConnection << inCommand << "\r\n" << flush;
  if( !mSocket.wait_for_read( static_cast<int>( 1e3 * mTimeout ) ) )
  {
    ostringstream oss;
    oss << "Command did not finish after timeout of " << mTimeout << " seconds";
    mResult = oss.str();
    return -1;
  }
  while( mConnection.rdbuf()->in_avail() )
  {
    char c = mConnection.get();
    if( c != '\r' )
      mResult += c;
  }
  int exitCode = 0;
  const string marker = "\\ExitCode";
  size_t pos = mResult.find( marker );
  if( pos != string::npos )
    istringstream( mResult.substr( pos + marker.length() ) ) >> exitCode;
  else
  { // Remove prompt which always forms the last line.
    pos = mResult.find_last_of( "\n" );
    if( pos == string::npos )
      pos = 0;
  }
  mResult = mResult.substr( 0, pos );
  return exitCode;
}

bool
BCI2000Connection::Run( const string& inOperatorPath, const string& inOptions )
{
  string options = inOptions;
  options += " --Telnet \"" + mTelnetAddress + "\"";
  options += " --StartupIdle";
  if( !mWindowTitle.empty() )
    options += " --Title \"" + mWindowTitle + "\"";
  if( !mWindowVisible )
    options += " --Hide";
  bool success = StartExecutable( inOperatorPath, options );
  if( success )
  {
    const int cReactionTime = 50; // ms
    double timeElapsed = 0; // s
    client_tcpsocket testSocket;
    while( timeElapsed < mTimeout && !testSocket.is_open() )
    {
      testSocket.open( mTelnetAddress.c_str() );
      testSocket.wait_for_read( cReactionTime );
      timeElapsed += 1e-3 * cReactionTime;
    }
    success = testSocket.is_open();
    if( !success )
      mResult = "Operator module does not listen on " + mTelnetAddress;
  }
  return success;
}

bool
BCI2000Connection::Quit()
{
  Execute( "quit" );
  return Result().empty();
}

#if _WIN32
bool
BCI2000Connection::StartExecutable( const string& inExecutable, const string& inOptions )
{
  mResult.clear();
  string commandLine = "\"" + inExecutable + "\" " + inOptions;

  char* pWorkingDir = new char[inExecutable.length() + 1];
  ::strcpy( pWorkingDir, inExecutable.c_str() );
  ::PathRemoveFileSpecA( pWorkingDir );
  if( *pWorkingDir == '\0' )
  {
    delete[] pWorkingDir;
    pWorkingDir = NULL;
  }

  const char* pExt = ::PathFindExtensionA( inExecutable.c_str() );
  bool isScript = ( 0 != ::stricmp( pExt, ".exe" ) );
  if( isScript )
    commandLine = "cmd /c call " + commandLine;

  STARTUPINFOA si = { 0 };
  si.cb = sizeof( si );
#if 0
  si.dwFlags = STARTF_USESHOWWINDOW;
  si.wShowWindow = SW_SHOWNOACTIVATE;
#endif
  PROCESS_INFORMATION pi = { 0 };
  bool success =( TRUE == ::CreateProcessA( NULL, const_cast<char*>( commandLine.c_str() ), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, pWorkingDir, &si, &pi ) );
  if( !success )
  {
    ostringstream oss;
    oss << "Could not execute: " << commandLine << "\n"
        << "in working directory: " << pWorkingDir << "\n"
        << "Error code: 0x" << hex << ::GetLastError();
    mResult = oss.str();
  }
  delete[] pWorkingDir;
  if( success )
  {
    DWORD exitCode = 0;
    ::GetExitCodeProcess( pi.hProcess, &exitCode );
    if( exitCode == STILL_ACTIVE )
    {
      ::WaitForInputIdle( pi.hProcess, static_cast<DWORD>( mTimeout * 1e3 ) );
      ::GetExitCodeProcess( pi.hProcess, &exitCode );
    }
    if( isScript )
    {
      success = ( exitCode == 0 );
      if( !success )
        mResult = "Execution of script \"" + inExecutable + "\" failed";
    }
    else if( exitCode != STILL_ACTIVE )
    {
      success = false;
      mResult = "Application initialization failed";
    }
    ::CloseHandle( pi.hProcess );
    ::CloseHandle( pi.hThread );
  }
  return success;
}
#else // _WIN32
bool
BCI2000Connection::StartExecutable( const string& inExecutable, const string& inOptions )
{
  mResult.clear();
  string commandLine = "\"" + inExecutable + "\" " + inOptions + " &";
  size_t pos = inExecutable.find_last_of( "/" );
  if( pos != string::npos )
  {
    string workingDir = workingDir.substr( 0, pos + 1 );
    commandLine = "cd \"" + workingDir + "\" && " + commandLine;
  }
  bool success = ( 0 == ::system( commandLine.c_str() ) );
  if( !success )
    mResult = "Could not execute: " + commandLine;
  return success;
}
#endif // _WIN32
