////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Declarations for stream symbols related to error handling.
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

#include "BCIError.h"
#include "OSThread.h"

using namespace std;

// Make sure ios_base is properly initialized before our OutStreams are
// constructed.
static ios_base::Init ios_base_Init_;

// Definitions of the actual global objects.
BCIError::OutStream bcierr___( BCIError::RuntimeError );
BCIError::OutStream bciout___( BCIError::Warning );
BCIError::OutStream bcidbg___( BCIError::DebugMessage );

namespace BCIError
{

list<string> OutStream::sContext;
int OutStream::sDebugLevel = 0;

OutStream&
OutStream::operator()( const string& inContext )
{
  if( !OSThread::IsMainThread() || sContext.empty() )
    mBuf.SetContext( inContext );
  else
    ( *this )();
  return *this;
}

OutStream&
OutStream::operator()()
{
  mBuf.SetContext( sContext );
  return *this;
}

OutStream&
OutStream::Debug( int inDebugLevel )
{
  if( sDebugLevel >= inDebugLevel )
    rdbuf( &mBuf );
  else
    rdbuf( NULL );
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
  if( str().length() > 1 )
  {
    string message = mContext + str();
    str( "" );
    size_t pos;
    while( ( pos = message.find( '\0' ) ) != string::npos )
      message = message.substr( 0, pos ) + message.substr( pos + 1 );
    if( mpOnFlush )
      mpOnFlush( message );
    else if( f )
      f( message );
  }
  else
  {
    str( "" );
  }
  FlushHandler h = mpOnFlush;
  mpOnFlush = f;
  return h;
}

int
OutStream::StringBuf::sync()
{
  int r = stringbuf::sync();
  ++mNumFlushes;
  SetFlushHandler( mpOnFlush );
  return r;
}

} // namespace BCIError

