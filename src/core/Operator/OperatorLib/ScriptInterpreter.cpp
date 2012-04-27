////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: A class that encapsulates interpretation of operator scripts.
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

#include "ScriptInterpreter.h"
#include "StateMachine.h"
#include "BCI_OperatorLib.h"
#include "HybridString.h"
#include "BCIException.h"
#include "ObjectType.h"
#include <climits>

using namespace std;
using namespace Interpreter;

// ScriptInterpreter definitions

ScriptInterpreter::ScriptInterpreter( class StateMachine& inStateMachine )
: mLine( 0 ),
  mrStateMachine( inStateMachine )
{
  mrStateMachine.AddListener( *this );
}

ScriptInterpreter::~ScriptInterpreter()
{
  mrStateMachine.RemoveListener( *this );
}

void
ScriptInterpreter::Initialize( class StateMachine& inStateMachine )
{
  ObjectType::Initialize( inStateMachine );
}

bool
ScriptInterpreter::Execute( const char* inScript )
{
  mLine = 0;
  bool syntaxOK = true;
  istringstream iss( inScript );
  while( !iss.eof() )
  {
    ++mLine;
    string line;
    std::getline( iss, line );
    syntaxOK = syntaxOK && ExecuteLine( line );
  }
  return syntaxOK;
}

bool
ScriptInterpreter::ExecuteLine( const string& inLine )
{
  if( inLine.empty() || inLine[0] == '#' )
    return true;

  bool syntaxOK = true;
  istringstream iss( inLine );
  while( !iss.eof() )
  {
    iss >> ws;
    streampos startpos = iss.tellg();
    HybridString().ReadUntil( iss, bind2nd( equal_to<int>(), ';' ) );
    streamoff length = iss.tellg() - startpos;
    iss.ignore();
    string command = iss.str().substr( startpos, length );
    syntaxOK = syntaxOK && ExecuteCommand( command );
  }
  return syntaxOK;
}

bool
ScriptInterpreter::ExecuteCommand( const string& inCommand )
{
  bool success = false;
  try
  {
    mResultStream.clear();
    mResultStream.str( "" );
    mInputStream.clear();
    mInputStream.str( inCommand );
    while( !mPosStack.empty() )
      mPosStack.pop();
    mPosStack.push( 0 );

    string verb = GetToken();
    if( !verb.empty() )
    {
      string type = GetToken();
      ObjectType* pType = ObjectType::ByName( type );
      if( !pType )
      {
        pType = ObjectType::ByName( "" );
        Unget();
        if( !pType || !pType->Execute( verb, *this ) )
        {
          if( CallbackBase::OK == mrStateMachine.ExecuteCallback( BCI_OnUnknownCommand, inCommand.c_str() ) )
            mInputStream.ignore( INT_MAX );
          else
            throw bciexception_( "Don't know how to " << verb << " " << type );
        }
      }
      else if( !pType->Execute( verb, *this ) )
      {
        if( CallbackBase::OK == mrStateMachine.ExecuteCallback( BCI_OnUnknownCommand, inCommand.c_str() ) )
          mInputStream.ignore( INT_MAX );
        else if( type.empty() )
          throw bciexception_( "Cannot " << verb << " without an object" );
        else
          throw bciexception_( "Cannot " << verb << " " << pType->Name() << " objects" );
      }
      if( !mInputStream.eof() )
        throw bciexception_( "Extra argument" );
    }
    success = true;
  }
  catch( const BCIException& e )
  {
    ostringstream oss;
    if( mLine > 1 )
      oss << "Line " << mLine;
    else
      oss << inCommand;
    oss << ": " << e.what();
    OnScriptError( oss.str() );
  }
  return success;
}

void
ScriptInterpreter::OnScriptError( const string& inMessage )
{
  mrStateMachine.ExecuteCallback( BCI_OnScriptError, inMessage.c_str() );
}

string
ScriptInterpreter::GetToken()
{
  mPosStack.push( mInputStream.tellg() );
  HybridString result;
  mInputStream >> ws >> result;
  return result;
}

string
ScriptInterpreter::GetRemainder()
{
  mPosStack.push( mInputStream.tellg() );
  string result;
  std::getline( mInputStream >> ws, result, '\0' );
  return result;
}

void
ScriptInterpreter::Unget()
{
  if( mPosStack.empty() )
    throw bciexception_( "Cannot unget" );
  mInputStream.seekg( mPosStack.top() );
  mPosStack.pop();
}

void
ScriptInterpreter::ParseArguments( string& ioFunction, ArgumentList& outArgs )
{
  outArgs.clear();
  size_t pos1 = ioFunction.find( '(' );
  string name = ioFunction.substr( 0, pos1 );
  while( pos1 != string::npos )
  {
    size_t pos2 = ioFunction.find( ')', pos1 + 1 );
    if( pos2 == string::npos )
      throw bciexception_( ioFunction << ": missing ')'" );
    istringstream args( ioFunction.substr( pos1 + 1, pos2 - pos1 - 1 ) );
    vector<string> arglist;
    string arg;
    while( std::getline( args >> ws, arg, ',' ) )
      arglist.push_back( arg );
    outArgs.push_back( arglist );
    pos1 = ioFunction.find( '(', pos2 );
  }
  ioFunction = name;
}

void
ScriptInterpreter::HandleLogMessage( int inMessageCallbackID, const std::string& inMessage )
{
  OSMutex::Lock lock( mMutex );
  if( mLogCapture.find( inMessageCallbackID ) != mLogCapture.end() )
  {
    switch( inMessageCallbackID )
    {
      case BCI_OnErrorMessage:
        mLogBuffer.append( "Error: " );
        break;
      case BCI_OnWarningMessage:
        mLogBuffer.append( "Warning: " );
        break;
    }
    mLogBuffer.append( inMessage ).append( "\n" );
  }
}

// LogStream::LogBuffer definitions
int
ScriptInterpreter::LogStream::LogBuffer::sync()
{
  mrStateMachine.LogMessage( BCI_OnLogMessage, str().c_str() );
  return 0;
}
