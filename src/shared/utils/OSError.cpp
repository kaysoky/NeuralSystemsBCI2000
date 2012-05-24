/////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class for OS error codes and messages.
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
#include "PCHIncludes.h"
#pragma hdrstop

#include "OSError.h"

#ifdef _WIN32
#include "Windows.h"
#endif

using namespace std;

const string OSError::cDefaultMessage = "<n/a>";

OSError::OSError()
: mCode( 0 ),
  mMessage( cDefaultMessage )
{
#ifdef _WIN32
  mCode = ::GetLastError();
#endif
};

const char*
OSError::Message() const
{
  mMessage = cDefaultMessage;
#ifdef _WIN32
  char* pMessage = NULL;
  bool success = ::FormatMessageA(
    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
    NULL,
    mCode,
    MAKELANGID( LANG_ENGLISH, SUBLANG_DEFAULT ),
    reinterpret_cast<char*>( *pMessage ),
    0,
    NULL
  );
  if( success )
  {
    mMessage = pMessage;
    ::LocalFree( pMessage );
  }
#endif // _WIN32
  return mMessage.c_str();
}
