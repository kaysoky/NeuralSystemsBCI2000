/////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class for OS error codes and messages.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
/////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "OSError.h"

#ifdef _WIN32
#include "Windows.h"
#endif

using namespace std;

char* OSError::spMessageBuffer = NULL;

OSError::OSError()
: mCode( 0 )
{
#ifdef _WIN32
  mCode = ::GetLastError();
#endif
};

const char*
OSError::Message() const
{
  delete[] spMessageBuffer;
  spMessageBuffer = NULL;
#ifdef _WIN32
  char* pMessage;
  bool success = ::FormatMessage(
    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
    NULL,
    mCode,
    MAKELANGID( LANG_ENGLISH, SUBLANG_DEFAULT ),
    reinterpret_cast<LPTSTR>( &pMessage ),
    0,
    NULL
  );
  if( success )
  {
    spMessageBuffer = new char[ ::strlen( pMessage ) + 1 ];
    ::strcpy( pMessage, spMessageBuffer );
    ::LocalFree( pMessage );
  }
  else
  {
    spMessageBuffer = new char[ 1 ];
    spMessageBuffer[ 0 ] = '\0';
  }
#endif // _WIN32
  return spMessageBuffer;
}
