////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: juergen.mellinger@uni-tuebingen.de
// Description: A simple telnet server that runs inside a BCI2000
//   operator module.
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

#include "TelnetServer.h"
#include "VersionInfo.h"
#include "BCIException.h"

#include <string>
#include <iostream>

using namespace std;

TelnetServer::TelnetServer( class StateMachine& inStateMachine, const string& inAddress )
: ScriptInterpreter( inStateMachine ),
  mpChild( NULL ),
  mpParent( NULL ),
  mAddress( inAddress ),
  mpListeningSocket( new server_tcpsocket ),
  mStream( mSocket )
{
  mpListeningSocket->open( inAddress.c_str() );
  if( !mpListeningSocket->is_open() )
    throw bciexception_( "TelnetServer: Cannot listen at " << mAddress );
  Initialize();
}

TelnetServer::TelnetServer( TelnetServer* pParent )
: ScriptInterpreter( *pParent ),
  mpChild( NULL ),
  mpParent( pParent ),
  mAddress( pParent->mAddress ),
  mpListeningSocket( pParent->mpListeningSocket ),
  mStream( mSocket )
{
  Initialize();
}

void
TelnetServer::Initialize()
{
  ScriptInterpreter::WriteLineHandler( &OnWriteLine, this );
  ScriptInterpreter::ReadLineHandler( &OnReadLine, this );
  OSThread::Start();
}

TelnetServer::~TelnetServer()
{
  ScriptInterpreter::Abort();
  if( !OSThread::InOwnThread() )
    OSThread::TerminateWait();
  OSMutex::Lock lock( mMutex );
  delete mpChild;
}

int
TelnetServer::OnExecute()
{
  const int cReactionTimeMs = 100;
  while( !OSThread::IsTerminating() && !mpListeningSocket->wait_for_accept( mSocket, cReactionTimeMs ) )
    ;
  if( !OSThread::IsTerminating() )
  {
    mpChild = new TelnetServer( this );
    WriteHello().WriteNewline().WritePrompt();
  }
  string line;
  while( mStream && mSocket.connected() && !OSThread::IsTerminating() )
  {
    while( mStream && mStream.rdbuf()->in_avail() )
      ReadCharacter();
    mSocket.wait_for_read( cReactionTimeMs );
  }
  return 0;
}

void
TelnetServer::OnFinished()
{
  // When the connection has been closed from the other side,
  // remove ourselves from the chain of TelnetServer instances,
  // and delete ourselves.
  if( !OSThread::IsTerminating() && mpParent )
  {
    {
      OSMutex::Lock lock( mpParent->mMutex );
      if( mpChild )
      {
        OSMutex::Lock lock( mpChild->mMutex );
        mpChild->mpParent = mpParent;
        mpParent->mpChild = mpChild;
        mpChild = NULL;
      }
      else
      {
        mpParent->mpChild = NULL;
      }
    }
    delete this;
  }
}

void
TelnetServer::OnScriptError( const string& inMessage )
{
  Write( inMessage ).WriteNewline();
  ScriptInterpreter::OnScriptError( inMessage );
}

bool
TelnetServer::OnWriteLine( void* inInstance, const string& inLine )
{
  TelnetServer* this_ = reinterpret_cast<TelnetServer*>( inInstance );
  this_->Write( inLine ).WriteNewline();
  return this_->mStream << flush;
}

bool
TelnetServer::OnReadLine( void* inInstance, string& outLine )
{
  TelnetServer* this_ = reinterpret_cast<TelnetServer*>( inInstance );
  this_->Write( "\\AwaitingInput:" ).WriteNewline();
  bool result = getline( this_->mStream, outLine );
  if( result )
    this_->Write( "\\AcknowledgedInput" ).WriteNewline();
  if( !outLine.empty() && *outLine.rbegin() == '\r' )
    outLine = outLine.substr( 0, outLine.length() - 1 );
  return result;
}

TelnetServer&
TelnetServer::ReadCharacter()
{
  int c = mStream.get();
  switch( c )
  {
    case '\n':
      if( ScriptInterpreter::Execute( mLineBuffer ) )
      {
        Write( ScriptInterpreter::Result() );
        if( !ScriptInterpreter::Result().empty()
            && *ScriptInterpreter::Result().rbegin() != '\n' )
          WriteNewline();
      }
      WritePrompt();
      mLineBuffer.clear();
      break;
    case '\r':
    case '\x07': // Bell
      break;
    case '\x08': // Backspace
      if( !mLineBuffer.empty() )
        mLineBuffer = mLineBuffer.substr( 0, mLineBuffer.length() - 1 );
      break;
    case 255: // Interpret As Command
      switch( mStream.get() )
      {
        case 247: // Erase Character
          if( !mLineBuffer.empty() )
            mLineBuffer = mLineBuffer.substr( 0, mLineBuffer.length() - 1 );
          break;
        case 248: // Erase Line
          mLineBuffer.clear();
          break;
        default:
          while( mStream && mStream.rdbuf()->in_avail() && mStream.peek() > 127 )
            mStream.get();
      }
      break;
    default:
      mLineBuffer += c;
  }
  return *this;
}


TelnetServer&
TelnetServer::WriteHello()
{
  mStream << "BCI2000 Version " << VersionInfo::Current[VersionInfo::VersionID];
  char hostname[512] = "";
  ::gethostname( hostname, sizeof( hostname ) );
  if( *hostname )
    mStream << " on " << hostname;
  WriteNewline();
  mStream << "Type 'help' for a list of commands.";
  return *this;
}

TelnetServer&
TelnetServer::Write( const string& inString )
{
  for( string::const_iterator i = inString.begin(); i != inString.end(); ++i )
  {
    switch( *i )
    {
      case '\n':
        WriteNewline();
        break;
      default:
        mStream.put( *i );
    }
  }
  return *this;
}

TelnetServer&
TelnetServer::WriteNewline()
{
  mStream.put( '\r' ).put( '\n' );
  return *this;
}

TelnetServer&
TelnetServer::WritePrompt()
{
  mStream.put( '>' ).flush();
  return *this;
}
