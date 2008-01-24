////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Declarations for stream symbols related to error handling.
//              To report an error, write e.g.
//               bcierr << "My error message" << endl;
//              For an informational message, write
//               bciout << "My info message" << endl;
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "BCIError.h"

using namespace std;

// Make sure ios_base is properly initialized before our OutStreams are
// constructed.
static ios_base::Init ios_base_Init_;

// Definitions of the actual global objects.
BCIError::OutStream bcierr__;
BCIError::OutStream bciout__;
BCIError::OutStream bcidbg__;

namespace BCIError
{

string OutStream::sContext = "";
int    OutStream::sDebugLevel = 0;

OutStream&
OutStream::operator()( const char* inContext )
{
  if( sContext.empty() )
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
OutStream::StringBuf::SetFlushHandler( OutStream::FlushHandler f )
{
  if( str().length() > 1 )
  {
    string message = mContext.empty() ? str() : mContext + ": " + str();
    if( mpOnFlush )
      mpOnFlush( message );
    else
      f( message );
  }
  str( "" );
  mpOnFlush = f;
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