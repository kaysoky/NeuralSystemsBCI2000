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
#include "BCITest.h"
#include "defines.h"

#if _WIN32
# include <Windows.h>
#else // _WIN32
# include <locale>
#endif // _WIN32

#include <cstring>
#include <ctime>
#include <cstdlib>
#include <sstream>
#include <functional>

using namespace std;

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

const string StringUtils::WhiteSpace = " \n\t\r";

string
StringUtils::LStrip( const string& s, const string& chars )
{
  size_t pos = s.find_first_not_of( chars );
  return pos == string::npos ? "" : s.substr( pos );
}

string
StringUtils::RStrip( const string& s, const string& chars )
{
  size_t pos = s.find_last_not_of( chars );
  return pos == string::npos ? "" : s.substr( 0, pos + 1 );
}

string
StringUtils::Strip( const string& s, const string& chars )
{
  return RStrip( LStrip( s, chars ), chars );
}

wstring
StringUtils::ToUpper( const wstring& s )
{
  wstring result = s;
  for( wstring::iterator i = result.begin(); i != result.end(); ++i )
    *i = ::towupper( *i );
  return result;
}

wstring
StringUtils::ToLower( const wstring& s )
{
  wstring result = s;
  for( wstring::iterator i = result.begin(); i != result.end(); ++i )
    *i = ::towlower( *i );
  return result;
}

static const char cBase64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
static const uint8_t cBase64Fill = 64;
static uint8_t cInvBase64[256] = "";

static int
InitBase64()
{
  for( size_t i = 0; i < sizeof( cInvBase64 ); ++i )
    cInvBase64[i] = 255;
  for( uint8_t i = 0; i < sizeof( cBase64 ); ++i )
    cInvBase64[cBase64[i]] = i;
  return 0;
}
static int sInitBase64 = InitBase64();

ostream&
StringUtils::WriteAsBase64( ostream& os, const string& s )
{
  const uint32_t mask64 = ( 1 << 6 ) - 1;
  size_t pos = 0;
  uint32_t triplet = 0;
  bool done = false,
       atEnd = false;
  int count = 0,
      rem = 0;
  while( !done )
  {
    if( pos >= s.length() )
    {
      atEnd = true;
      done = ( count == 0 );
    }
    uint32_t c = atEnd ? 0 : static_cast<uint8_t>( s[pos++] );
    if( atEnd )
      ++rem;
    triplet <<= 8;
    triplet |= c;
    ++count %= 3;
    if( 0 == count )
    {
      for( int i = 3; i >= rem; --i )
        os.put( cBase64[( triplet >> ( 6 * i ) ) & mask64] );
      for( int i = 0; i < rem; ++i )
        os.put( cBase64[cBase64Fill] );
      triplet = 0;
    }
  }
  return os;
}

namespace {
template<typename T>
istream&
ReadAsBase64_( istream& is, string& s, T stopIf )
{
  s.clear();
  const uint32_t mask256 = ( 1 << 8 ) - 1;
  bool done = false,
       atEnd = false;
  uint32_t triplet = 0;
  int rem = 0, count = 0;
  while( !done )
  {
    int c = cBase64[cBase64Fill];
    if( !atEnd )
    {
      c = is.get();
      if( stopIf( c ) || c >= 256 || c < 0 )
      {
        atEnd = true;
        done = ( count == 0 );
      }
    }
    uint32_t code = cInvBase64[c];
    if( code == cBase64Fill )
    {
      ++rem;
      code = 0;
    }
    if( code < cBase64Fill )
    {
      triplet <<= 6;
      triplet |= code;
      if( ++count == 4 )
      {
        for( int i = 2; i >= rem; --i )
          s += static_cast<char>( triplet >> ( 8 * i ) & mask256 );
        count = 0;
        rem = 0;
        triplet = 0;
        done = atEnd;
      }
    }
  }
  return is;
}
} // namespace

istream&
StringUtils::ReadAsBase64( std::istream& is, std::string& s, int (*stopIf)( int ) )
{
  static struct { bool operator()( int ) { return false; } } constFalse;
  return stopIf ? ReadAsBase64_( is, s, stopIf ) : ReadAsBase64_( is, s, constFalse );
}

istream&
StringUtils::ReadAsBase64( std::istream& is, std::string& s, int stopAt )
{
  return ReadAsBase64_( is, s, std::bind1st( std::equal_to<int>(), stopAt ) );
}

bcitest( Base64Test )
{
  unsigned int seed = static_cast<unsigned int>( ::time( NULL ) );
  ::srand( seed );
  for( int i = 0; i < 100; ++i )
  {
    string s;
    while( rand() < ( 99 * RAND_MAX / 100 ) )
      s += static_cast<char>( rand() % 256 );
    stringstream stream;
    StringUtils::WriteAsBase64( stream, s );
    string s2;
    StringUtils::ReadAsBase64( stream, s2 );
    bcifail_if( s != s2, "seed: " << seed << ", iteration: " << i );
  }
  ::srand( 1 );
}
