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
#include "VariableTypes.h"
#include "BCIAssert.h"
#include <climits>
#include <ctime>

using namespace std;
using namespace Interpreter;

CommandInterpreter::CommandInterpreter( class StateMachine& inStateMachine )
: mrStateMachine( inStateMachine ),
  mpParent( NULL ),
  mAbort( false ),
  mWriteLineFunc( &OnWriteLineDefault ),
  mpWriteLineData( this ),
  mReadLineFunc( NULL ),
  mpReadLineData( NULL )
{
  Init();
}

CommandInterpreter::CommandInterpreter( CommandInterpreter& inOther )
: mrStateMachine( inOther.mrStateMachine ),
  mpParent( &inOther ),
  mExpressionVariables( inOther.mExpressionVariables ),
  mLocalVariables( inOther.mLocalVariables ),
  mAbort( false ),
  mWriteLineFunc( inOther.mWriteLineFunc ),
  mpWriteLineData( inOther.mpWriteLineData ),
  mReadLineFunc( inOther.mReadLineFunc ),
  mpReadLineData( inOther.mpReadLineData )
{
  Init();
}

void
CommandInterpreter::Init()
{
  if( mpParent )
  {
    mpParent->AddListener( *this );
  }
  else
  {
    ObjectType::Initialize( mrStateMachine );
    mrStateMachine.AddListener( *this );
  }
  mLocalVariables[ResultName()] = "";

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
  if( mpParent )
    mpParent->RemoveListener( *this );
  else
    mrStateMachine.RemoveListener( *this );
}

void
CommandInterpreter::AddListener( CommandInterpreter& inListener )
{
  mListeners.insert( &inListener );
}

void
CommandInterpreter::RemoveListener( CommandInterpreter& inListener )
{
  mListeners.erase( &inListener );
}

int
CommandInterpreter::Execute( const string& inCommand )
{
  Background();

  mResultStream.clear();
  mResultStream.str( "" );
  mInputStream.clear();
  mInputStream.str( inCommand );
  mInputStream >> ws;
  while( !mPosStack.empty() )
    mPosStack.pop();

  string verb = GetToken();
  if( !verb.empty() )
  {
    string type = GetOptionalToken();
    ObjectType* pType = ObjectType::ByName( type );
    bool success = ( pType && pType->Execute( verb, *this ) );
    if( !success )
    {
      success = ( CallbackBase::OK == mrStateMachine.ExecuteCallback( BCI_OnUnknownCommand, inCommand.c_str() ) );
      if( success )
        GetOptionalRemainder();
    }
    if( !success )
    {
      Unget();
      pType = ObjectType::ByName( "" );
      if( !pType )
        throw bciexception( "No implied type available" );
      success = pType->Execute( verb, *this );
    }
    if( !success )
    {
      mInputStream.clear();
      mInputStream.seekg( 0 );
      mInputStream >> ws;
      success = ImpliedType::Get( *this );
    }
    if( !success )
    {
      if( type.empty() || !::isalpha( type[0] ) )
        throw bciexception_( "Cannot make sense of \"" << inCommand << "\"" );
      else if( *pType->Name() != '\0' )
        throw bciexception_( "Cannot " << verb << " " << pType->Name() << " objects" );
      else
        throw bciexception_( "Don't know how to " << verb << " " << type );
    }
    if( InputFailed() )
      throw bciexception_( "Missing argument" );
    mInputStream.clear();
    if( mInputStream.tellg() != static_cast<streampos>( inCommand.length() ) )
      throw bciexception_( "Extra argument" );
  }
  mLocalVariables[ResultName()] = mResultStream.str();
  return EvaluateResult( inCommand );
}

string
CommandInterpreter::SubstituteCommands( const string& input )
{
  string output, command;
  string::const_iterator i = input.begin();
  int braceLevel = 0;
  while( i != input.end() )
  {
    string& appendTo = ( braceLevel > 0 ) ? command : output;
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
            appendTo += '$';
            appendTo += *i;
          }
          ++braceLevel;
        }
        else
        {
          string name;
          while( i != input.end() && ::isalnum( *i ) )
            name += *i++;
          --i;
          if( name.empty() )
            appendTo += '$';
          else
          {
            ParserToken value = GetVariable( name );
            ostringstream oss;
            oss << value;
            appendTo += oss.str();
          }
        }
        break;

      case '{':
        if( braceLevel > 0 )
          ++braceLevel;
        appendTo += *i;
        break;

      case '}':
        if( braceLevel == 1 )
        {
          Execute( SubstituteCommands( command ) );
          ParserToken result = Result();
          command.clear();
          while( result.length() > 0 && !::isprint( *result.rbegin() ) )
            result = result.substr( 0, result.length() - 1 );
          ostringstream oss;
          oss << result;
          output += oss.str();
        }
        else
          appendTo += *i;
        if( braceLevel > 0 )
          --braceLevel;
        break;

      default:
        appendTo += *i;
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

string
CommandInterpreter::GetVariable( const string& inName )
{
  string result;
  if( mExpressionVariables.find( inName ) != mExpressionVariables.end() )
  {
    ostringstream oss;
    oss << mExpressionVariables[inName];
    result = oss.str();
  }
  else if( mLocalVariables.Exists( inName ) )
    result = mLocalVariables[inName];
  else
    EnvVariable::Get( inName, result );
  return result;
}

int
CommandInterpreter::EvaluateResult( const string& inCommand )
{
  string result = Result();

  if( result.empty() )
    return 0;

  if( !::stricmp( result.c_str(), "true" ) )
    return 0;

  double number;
  istringstream iss( result );
  if( ( iss >> number ) && iss.eof() )
    return number != 0 ? 0 : 1;

  static const string tag = ExitCodeTag();
  size_t pos = result.find( tag );
  if( pos != string::npos )
  {
    istringstream exitStream( result.substr( pos + tag.length() ) );
    int exitCode;
    if( exitStream >> exitCode )
      return exitCode;
  }

  // For some commands, the absence of an exit code indicates success.
  static const string specialCommands[] =
  { "system ", "start executable ", };
  for( size_t i = 0; i < sizeof( specialCommands ) / sizeof( *specialCommands ); ++i )
    if( !::stricmp( specialCommands[i].c_str(), inCommand.substr( 0, specialCommands[i].length() ).c_str() ) )
      return 0;

  return 1;
}

int
CommandInterpreter::Background( int inSleepTime )
{
  if( mAbort )
  {
    mAbort = false;
    throw bciexception_( "Script execution aborted" );
  }
  ThreadUtils::SleepFor( inSleepTime );
  return inSleepTime;
}

string
CommandInterpreter::GetToken()
{
  mInputStream.clear();
  mPosStack.push( mInputStream.tellg() );
  ParserToken token;
  mInputStream >> token >> ws;
  return token;
}

string
CommandInterpreter::GetOptionalToken()
{
  string result = GetToken();
  if( InputFailed() )
    Unget();
  return result;
}

string
CommandInterpreter::GetRemainder()
{
  mInputStream.clear();
  mPosStack.push( mInputStream.tellg() );
  string result;
  std::getline( mInputStream >> ws, result, '\0' );
  return result;
}

string
CommandInterpreter::GetOptionalRemainder()
{
  string result = GetRemainder();
  if( InputFailed() )
    Unget();
  return result;
}

void
CommandInterpreter::Unget()
{
  if( mPosStack.empty() )
    throw bciexception_( "Cannot unget" );
  mInputStream.clear();
  mInputStream.seekg( mPosStack.top() );
  mPosStack.pop();
}

bool
CommandInterpreter::InputFailed()
{
  mInputStream.clear();
  return !mPosStack.empty() && mPosStack.top() == mInputStream.tellg();
}

bool
CommandInterpreter::WriteLine( const std::string& inLine )
{
  bool result = false;
  if( mWriteLineFunc )
    result = mWriteLineFunc( mpWriteLineData, inLine );
  return result;
}

bool
CommandInterpreter::ReadLine( std::string& outLine )
{
  bool result = false;
  if( mReadLineFunc )
    result = mReadLineFunc( mpReadLineData, outLine );
  return result;
}

CommandInterpreter&
CommandInterpreter::WriteLineHandler( WriteLineFunc inFunc, void* inData )
{
  mWriteLineFunc = inFunc;
  mpWriteLineData = inData;
  return *this;
}

CommandInterpreter&
CommandInterpreter::ReadLineHandler( ReadLineFunc inFunc, void* inData )
{
  mReadLineFunc = inFunc;
  mpReadLineData = inData;
  return *this;
}

bool
CommandInterpreter::OnWriteLineDefault( void* inData, const std::string& inLine )
{
  CommandInterpreter* pInterpreter = reinterpret_cast<CommandInterpreter*>( inData );
  pInterpreter->Log() << inLine;
  return true;
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
CommandInterpreter::Abort()
{
  mAbort = true;
  for( set<CommandInterpreter*>::const_iterator i = mListeners.begin(); i != mListeners.end(); ++i )
    ( *i )->Abort();
}

void
CommandInterpreter::HandleLogMessage( int inMessageCallbackID, const std::string& inMessage )
{
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
  for( set<CommandInterpreter*>::const_iterator i = mListeners.begin(); i != mListeners.end(); ++i )
    ( *i )->HandleLogMessage( inMessageCallbackID, inMessage );
}

// LogStream::LogBuffer definitions
int
CommandInterpreter::LogStream::LogBuffer::sync()
{
  mrStateMachine.LogMessage( BCI_OnLogMessage, str().c_str() );
  return 0;
}
