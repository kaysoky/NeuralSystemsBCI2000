//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: String-conversion related utility functions.
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
///////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "StringUtils.h"

#if _WIN32
# include <Windows.h>
#else // _WIN32
# include <locale>
#endif // _WIN32
#include <cstring>

using namespace std;
using namespace StringUtils;

wstring
StringUtils::ToWide( const char* inString )
{
#if _WIN32
  const int count = ::MultiByteToWideChar( CP_ACP, 0, inString, -1, NULL, 0 );
  wchar_t* pBuffer = new wchar_t[count];
  ::MultiByteToWideChar( CP_ACP, 0, inString, -1, pBuffer, count );
  wstring result( pBuffer );
  delete[] pBuffer;
#else // _WIN32
  locale loc;
  size_t length = ::strlen( inString );
  wchar_t* pBuffer = new wchar_t[length + 1];
  pBuffer[length] = 0;
  use_facet< ctype<wchar_t> >( loc ).widen( inString, inString + length, pBuffer );
  wstring result( pBuffer );
  delete[] pBuffer;
#endif // _WIN32
  return result;
}

string
StringUtils::ToNarrow( const wchar_t* inString )
{
#if _WIN32
  const int count = ::WideCharToMultiByte( CP_ACP, 0, inString, -1, NULL, 0, NULL, NULL );
  char* pBuffer = new char[count];
  ::WideCharToMultiByte( CP_ACP, 0, inString, -1, pBuffer, count, NULL, NULL );
  string result( pBuffer );
  delete[] pBuffer;
#else // _WIN32
  locale loc;
  size_t length = ::wcslen( inString );
  char* pBuffer = new char[length + 1];
  pBuffer[length] = 0;
  use_facet< ctype<wchar_t> >( loc ).narrow( inString, inString + length, '?', pBuffer );
  string result( pBuffer );
  delete[] pBuffer;
#endif // _WIN32
  return result;
}

