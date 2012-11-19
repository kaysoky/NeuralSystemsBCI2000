////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Declarations for stream symbols related to error/info messages.
//              To report an error, write e.g.
//               bcierr << "My error message" << endl;
//              For an informational message, write
//               bciout << "My info message" << endl;
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

#include "BCIStream.h"
#include "ThreadUtils.h"
#include "ParamList.h"
#include "ParamRef.h"

using namespace std;
using namespace BCIStream;

// Make sure ios_base is properly initialized before our OutStreams are
// constructed.
static ios_base::Init ios_base_Init_;

// Definitions of the actual global objects.
OutStream bcierr___( &RuntimeError );
OutStream bciwarn___( &Warning );
OutStream bciout___( &PlainMessage, 0 );
OutStream bcidbg___( &DebugMessage, 0 );

list<string> OutStream::sContext;

void
BCIStream::Apply( const ParamList& p )
{
  if( p.Exists( "Verbosity" ) )
  {
    int v = p( "Verbosity" );
    bciout.SetVerbosity( v );
    bciwarn.SetVerbosity( v );
  }
  if( p.Exists( "DebugLevel" ) )
    bcidbg.SetVerbosity( p( "DebugLevel" ) );
}

void
OutStream::MessageFilter( OutStream::FlushHandler& ioHandler, string& ioMessage )
{
  if( !ioMessage.empty() && *ioMessage.rbegin() == '\n' )
    ioMessage.erase( ioMessage.length() - 1 );
  if( !ioMessage.empty() && !::ispunct( *ioMessage.rbegin() ) )
    ioMessage += '.';

  if( ioHandler == &Warning || ioHandler == &PlainMessage )
  {
    static const string warning = "warning";
    if( !::stricmp( ioMessage.substr( 0, warning.length() ).c_str(), warning.c_str() ) )
    {
      size_t pos = warning.length();
      while( pos < ioMessage.length() && ( ::ispunct( ioMessage[pos] ) || ::isspace( ioMessage[pos] ) ) )
        ++pos;
      ioMessage = ioMessage.substr( pos );
      ioHandler = &Warning;
    }
  }
}

OutStream::OutStream( FlushHandler f, int level )
: std::ostream( 0 ),
  mVerbosityLevel( level ),
  mVerbosityLocked( false )
{
  this->init( &mBuf );
  mBuf.SetFlushHandler( f );
}

OutStream&
OutStream::operator()()
{
  mBuf.SetContext( sContext );
  return ResetFormat();
}


OutStream&
OutStream::operator()( const string& inContext )
{
  if( !ThreadUtils::InMainThread() || sContext.empty() )
    mBuf.SetContext( inContext );
  else
    mBuf.SetContext( sContext );
  return ResetFormat();
}

OutStream&
OutStream::ResetFormat()
{
  static ostringstream defaultFormat;
  this->copyfmt( defaultFormat );
  mVerbosityLocked = false;
  return *this;
}

OutStream&
OutStream::MessageVerbosity( int inLevel )
{
  if( !mVerbosityLocked )
  {
    if( mVerbosityLevel >= inLevel )
      rdbuf( &mBuf );
    else
      rdbuf( NULL );
    mVerbosityLocked |= ( inLevel == AlwaysDisplayMessage );
    mVerbosityLocked |= ( inLevel == NeverDisplayMessage );
  }
  return *this;
}

void
OutStream::SetContext( const std::string& s )
{
  if( s.empty() )
  {
    if( !OutStream::sContext.empty() )
      OutStream::sContext.pop_back();
  }
  else
    OutStream::sContext.push_back( s );
}

void
OutStream::StringBuf::SetContext( const list<string>& s )
{
  mContext.clear();
  if( !s.empty() )
    for( list<string>::const_iterator i = s.begin(); i != s.end(); ++i )
      mContext += *i + ": ";
}

void
OutStream::StringBuf::SetContext( const std::string& s )
{
  if( s.empty() )
    mContext.clear();
  else
    mContext = s + ": ";
}

OutStream::FlushHandler
OutStream::StringBuf::SetFlushHandler( OutStream::FlushHandler f )
{
  FlushHandler previous = mpOnFlush;
  bool empty = str().empty();
  if( previous && !empty )
    Flush();
  mpOnFlush = f;
  if( !previous && !empty )
    Flush();
  return previous;
}

void
OutStream::StringBuf::Flush()
{
  ++mNumFlushes;

  string s = str();
  str( "" );
  
  if( s.empty() || s == "\n" )
    s = "<empty message>";
  else for( size_t pos = s.find( '\0' ); pos != string::npos; pos = s.find( '\0', pos ) )
    s = s.substr( 0, pos ) + s.substr( pos + 1 );

  FlushHandler f = mpOnFlush;
  MessageFilter( f, s );
  if( f )
    f( mContext + s );
}

int
OutStream::StringBuf::sync()
{
  if( pptr() == pbase() )
    return 0;
    
  int r = stringbuf::sync();
  Flush();
  return r;
}
