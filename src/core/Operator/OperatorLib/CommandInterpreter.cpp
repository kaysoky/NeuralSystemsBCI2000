////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: A class that interprets individual operator scripting commands,
//   and performs command substitution.
//   The CommandInterpreter class implements three interfaces:
//   1) A client interface that provides an Execute() method, and access
//      to the last command's result.
//      The Execute() method is intentionally not thread safe. Use multiple
//      CommandInterpreter instances to execute scripting commands concurrently.
//   2) An internal "callback" interface to ObjectType descendant classes
//      which perform actual command execution.
//   3) A "listener" interface to the StateMachine class. This allows to
//      capture operator log messages.
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

#include "CommandInterpreter.h"
#include "StateMachine.h"
#include "BCI_OperatorLib.h"
#include "ParserToken.h"
#include "BCIException.h"
#include "ObjectType.h"
#include "ImpliedType.h"
#include "VariableType.h"
#include <climits>
#include <ctime>

using namespace std;
using namespace Interpreter;

CommandInterpreter::CommandInterpreter( class StateMachine& inStateMachine )
: mrStateMachine( inStateMachine ),
  mAbort( false )
{
  Init();
}

CommandInterpreter::CommandInterpreter( const CommandInterpreter& inOther )
: mrStateMachine( inOther.mrStateMachine ),
  mExpressionVariables( inOther.mExpressionVariables ),
  mAbort( false )
{
  Init();
}

void
CommandInterpreter::Init()
{
  ObjectType::Initialize( mrStateMachine );
  mrStateMachine.AddListener( *this );
  
  static const struct { const char* name, *format; }
  timevars[] = 
  {
    { "YYYYMMDD", "%Y%d%m" },
    { "HHMMSS", "%H%M%S" },
  };
  time_t t = ::time( NULL );
  struct tm* pTime = ::localtime( &t );
  char buffer[10] = "";
  for( size_t i = 0; i < sizeof( timevars ) / sizeof( *timevars ); ++i )
  {
    if( !::strftime( buffer, sizeof( buffer ) / sizeof( *buffer ), timevars[i].format, pTime ) )
      throw bciexception( "Could not format time variable: " << timevars[i].name );
    mLocalVariables[timevars[i].name] = buffer;
  }
}

CommandInterpreter::~CommandInterpreter()
{
  mrStateMachine.RemoveListener( *this );
}

void
CommandInterpreter::Execute( const string& inCommand )
{
  string command = SubstituteCommands( inCommand );
  mResultStream.clear();
  mResultStream.str( "" );
  mInputStream.clear();
  mInputStream.str( command );
  while( !mPosStack.empty() )
    mPosStack.pop();
  mPosStack.push( 0 );

  string verb = GetToken();
  if( !verb.empty() )
  {
    string type = GetToken();
    if( mInputStream.fail() )
      Unget();
    ObjectType* pType = ObjectType::ByName( type );
    if( !pType )
    {
      pType = ObjectType::ByName( "" );
      if( !pType )
        throw bciexception( "No implied type available" );
      Unget();
    }
    if( !pType->Execute( verb, *this ) )
    {
      if( CallbackBase::OK == mrStateMachine.ExecuteCallback( BCI_OnUnknownCommand, command.c_str() ) )
      {
        mInputStream.ignore( INT_MAX );
      }
      else
      {
        mInputStream.clear();
        mInputStream.seekg( 0 );
        mPosStack.push( 0 );
        if( !ImpliedType::Get( *this ) )
        {
          if( type.empty() || !::isalpha( type[0] ) )
            throw bciexception_( "Cannot make sense of \"" << command << "\"" );
          else if( *pType->Name() != '\0' )
            throw bciexception_( "Cannot " << verb << " " << pType->Name() << " objects" );
          else
            throw bciexception_( "Don't know how to " << verb << " " << type );
        }
      }
    }
    if( mInputStream.fail() )
      throw bciexception_( "Missing argument" );
    if( !mInputStream.eof() )
      throw bciexception_( "Extra argument" );
  }
  Background();
}

string
CommandInterpreter::SubstituteCommands( const string& input )
{
  string output, command;
  string::const_iterator i = input.begin();
  int braceLevel = 0;
  while( i != input.end() )
  {
    switch( *i )
    {
      case '$':
        ++i;
        if( i == input.end() )
          output += '$';
        else if( *i == '{' )
        {
          if( braceLevel > 0 )
          {
            output += '$';
            output += *i;
          }
          ++braceLevel;
        }
        else
        {
          output += '$';
          output += *i;
        }
        break;

      case '{':
        if( braceLevel > 0 )
          ++braceLevel;
        output += *i;
        break;

      case '}':
        if( braceLevel == 1 )
        {
          Execute( command );
          ParserToken result = Result();
          command.clear();
          while( result.length() > 0 && !::isprint( *result.rbegin() ) )
            result = result.substr( 0, result.length() - 1 );
          ostringstream oss;
          oss << result;
          output += oss.str();
        }
        else
          output += *i;
        if( braceLevel > 0 )
          --braceLevel;
        break;

      default:
        if( braceLevel > 0 )
          command += *i;
        else
          output += *i;
     }
     if( i != input.end() )
       ++i;
  }
  if( braceLevel > 0 )
  {
    output += "${";
    output += command;
  }
  return output;
}

int
CommandInterpreter::Background()
{
  if( mAbort )
  {
    mAbort = false;
    throw bciexception_( "Script execution aborted" );
  }
  const int sleepDuration = 1;
  ThreadUtils::SleepFor( sleepDuration );
  return sleepDuration;
}

string
CommandInterpreter::GetToken()
{
  mPosStack.push( mInputStream.tellg() );
  ParserToken token;
  mInputStream >> ws >> token >> ws;
  return token;
}

string
CommandInterpreter::GetRemainder()
{
  mPosStack.push( mInputStream.tellg() );
  string result;
  std::getline( mInputStream >> ws, result, '\0' );
  return result;
}

void
CommandInterpreter::Unget()
{
  if( mPosStack.empty() )
    throw bciexception_( "Cannot unget" );
  mInputStream.clear();
  mInputStream.seekg( mPosStack.top() );
  mInputStream.peek();
  mPosStack.pop();
}

void
CommandInterpreter::ParseArguments( string& ioFunction, ArgumentList& outArgs )
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
CommandInterpreter::HandleLogMessage( int inMessageCallbackID, const std::string& inMessage )
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
CommandInterpreter::LogStream::LogBuffer::sync()
{
  mrStateMachine.LogMessage( BCI_OnLogMessage, str().c_str() );
  return 0;
}
