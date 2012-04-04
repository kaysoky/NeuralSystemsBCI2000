/////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors juergen.mellinger@uni-tuebingen.de
// Description: A class that allows remote control of BCI2000.
//   Does not depend on the BCI2000 framework except for
//   src/shared/utils/SockStream.
//   On error, a function returns "false", and provides an error
//   message in the Result() property inherited from BCI2000Connection.
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
/////////////////////////////////////////////////////////////////////////////
#include "BCI2000Remote.h"
#include <fstream>
#include <sstream>

using namespace std;

BCI2000Remote&
BCI2000Remote::SubjectID( const std::string& inSubjectID )
{
  mSubjectID = inSubjectID;
  if( !inSubjectID.empty() )
    SimpleCommand( "set parameter SubjectName " + mSubjectID );
  return *this;
}

BCI2000Remote&
BCI2000Remote::SessionID( const std::string& inSessionID )
{
  mSessionID = inSessionID;
  if( !inSessionID.empty() )
    SimpleCommand( "set parameter SubjectSession " + mSessionID );
  return *this;
}

BCI2000Remote&
BCI2000Remote::DataDirectory( const std::string& inDataDirectory )
{
  mDataDirectory = inDataDirectory;
  if( !inDataDirectory.empty() )
    SimpleCommand( "set parameter DataDirectory " + mDataDirectory );
  return *this;
}

bool
BCI2000Remote::StartupModules( const std::vector<string>& inModules )
{
  Execute( "shutdown system" );
  bool success = WaitForSystemState( "Idle" );
  if( success )
  {
    ostringstream startupCommand;
    int port = 4000;
    startupCommand << "startup system ";
    for( size_t i = 0; i < inModules.size(); ++i )
      startupCommand << "module" << i + 1 << ":" << port++ << " ";
    Execute( startupCommand.str() );
    success = ( Result().find( "not" ) == string::npos );
  }
  if( success )
  {
    string errors;
    for( size_t i = 0; i < inModules.size(); ++i )
      if( Execute( "start executable " + inModules[i] ) != 0 )
        errors += "\n" + Result();
    success = errors.empty();
    if( !success )
      mResult = "Could not start modules: " + errors;
  }
  if( success )
    success = WaitForSystemState( "Initialization" );
  return success;
}

bool
BCI2000Remote::LoadParametersLocal( const string& inFileName )
{
  ifstream file( inFileName.c_str() );
  bool success = file.is_open();
  if( !success )
    mResult = "Could not open file \"" + inFileName + "\" for input.";
  if( success )
  {
    string line;
    while( std::getline( file, line ) )
      Execute( "set parameter " + line );
  }
  return success;
}

bool
BCI2000Remote::LoadParametersRemote( const string& inFileName )
{
  return SimpleCommand( "load parameters " + inFileName );
}

bool
BCI2000Remote::SetConfig()
{
  SubjectID( mSubjectID );
  SessionID( mSessionID );
  DataDirectory( mDataDirectory );
  bool success = SimpleCommand( "set config" );
  if( success )
    success = WaitForSystemState( "Resting" );
  return success;
}

bool
BCI2000Remote::Start()
{
  bool success = true;
  Execute( "get system state" );
  string state = Result();
  success = ( 0 != ::_stricmp( state.c_str(), "Running" ) );
  if( !success )
    mResult = "System is already in running state";
  if( success && 0 != ::_stricmp( state.c_str(), "Suspended" ) && 0 != ::_stricmp( state.c_str(), "Resting" ) )
    success = SetConfig();
  if( success )
    success = SimpleCommand( "start system" );
  if( success )
    success = WaitForSystemState( "Running" );
  return success;
}

bool
BCI2000Remote::Stop()
{
  Execute( "get system state" );
  bool success = !::_stricmp( Result().c_str(), "Running" );
  if( !success )
    mResult = "System is not in running state";
  if( success )
    success = SimpleCommand( "stop system" );
  return success;
}

bool
BCI2000Remote::GetSystemState( string& outResult )
{
  bool success = ( 0 == Execute( "get system state" ) );
  if( success )
    outResult = Result();
  else
    outResult.clear();
  return success;
}

bool
BCI2000Remote::SetStateVariable( const string& inStateName, double inValue )
{
  ostringstream value;
  value << inValue;
  Execute( "set state " + inStateName + " " + value.str() );
  return Result().empty();
}

bool
BCI2000Remote::GetStateVariable( const string& inStateName, double& outValue )
{
  Execute( "get state " + inStateName );
  return istringstream( Result() ) >> outValue;
}

bool
BCI2000Remote::GetControlSignal( int inChannel, int inElement, double& outValue )
{
  ostringstream oss;
  oss << "get signal(" << inChannel << "," << inElement << ")";
  Execute( oss.str() );
  return istringstream( Result() ) >> outValue;
}

bool
BCI2000Remote::WaitForSystemState( const string& inState )
{
  ostringstream timeout;
  timeout << Timeout() - 1;
  return SimpleCommand( "wait for " + inState + " " + timeout.str() );
}

bool
BCI2000Remote::SimpleCommand( const string& inCommand )
{
  Execute( inCommand );
  return Result().empty();
}

