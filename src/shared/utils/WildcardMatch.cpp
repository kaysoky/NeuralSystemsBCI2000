////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
//  Description: A function that matches strings against glob-like patterns.
//    In patterns, the following special characters are recognized:
//    "*" matches zero or more arbitrary characters,
//    "?" matches a single arbitrary character,
//    "[abc]" matches any of the characters "abc",
//    "[a-c]" matches any character from the range between "a" and "c",
//    "[!abc]" and "[!a-c]" both match any character not in "abc".
//    "\" is used as an escape character; write "\\" to match a single backslash.
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

#include "WildcardMatch.h"
#include "BCIException.h"

using namespace std;
using namespace bci;

namespace {

#if BCIDEBUG
const struct { const char* pattern, * match; }
sPositiveCases[] =
{
  { "*", "" },
  { "*String", "TestString" },
  { "Test*", "TestString" },
  { "T*tString", "TestString" },
  { "T*tString", "TtttString" },
  { "Tes?String", "TestString" },
  { "Test\?String", "Test?String" },
  { "?*estString", "TestString" },
  { "*TestString", "TestString" },
  { "*TestString*", "TestString" },
  { "TestString*", "TestString" },
  { "Te[s]tString", "TestString" },
  { "*[s]tString", "TestString" },
  { "*Str*", "TestString" },
  { "*TestString*", "TestString" },
  { "T*tring", "TestString" },
  { "*?String", "TestString" },
},
sNegativeCases[] =
{
  { "TestString", "" },
  { "", "TestString" },
  { "TestString", "Test" },
  { "?*TestString", "TestString" },
  { "*[!s]tString", "TestString" },
};

void RunTests()
{
  for( size_t i = 0; i < sizeof( sPositiveCases ) / sizeof( *sPositiveCases ); ++i )
    if( !WildcardMatch( sPositiveCases[i].pattern, sPositiveCases[i].match ) )
      throw bciexception(
        "WildcardMatch test case failed: \"" << sPositiveCases[i].pattern
        << "\" does not match \"" << sPositiveCases[i].match << "\""
      );
  for( size_t i = 0; i < sizeof( sNegativeCases ) / sizeof( *sNegativeCases ); ++i )
    if( WildcardMatch( sNegativeCases[i].pattern, sNegativeCases[i].match ) )
      throw bciexception(
        "WildcardMatch test case failed: \"" << sNegativeCases[i].pattern
        << "\" should not match \"" << sNegativeCases[i].match << "\""
      );
}
#endif // BCIDEBUG

bool Match( const char* p, const char* s, bool cs )
{
  bool result = false;
  switch( *p )
  {
    case '*':
      result = Match( p + 1, s, cs ) || ( *s != '\0' ) && ( Match( p + 1, s + 1, cs ) || Match( p, s + 1, cs ) );
      break;

    case '?':
      result = ( *s != '\0' ) && Match( p + 1, s + 1, cs );
      break;

    case '[':
    {
      ++p;
      string charset;
      bool negate = false;
      while( *p != '\0' && *p != ']' )
      {
        switch( *p )
        {
          case '!':
            if( charset.empty() )
              negate = true;
            else
              charset += *p;
            ++p;
            break;
          case '-':
            ++p;
            for( char c = *charset.rend(); c <= *p; ++c )
              charset += c;
            break;
          case '\\':
            if( *( p+1 ) != '\0' )
              ++p;
            /* no break */
          default:
            charset += *p++;
        }
        if( *p == '\0' )
        {
          result = false;
        }
        else
        {
          result = ( charset.find( *s ) != string::npos );
          if( negate )
            result = !result;
          result = result && Match( p + 1, s + 1, cs );
        }
      }
      break;
    }

    case '\0':
      result = ( *s == '\0' );
      break;

    case '\\':
      result = ( *( p + 1 ) != '\0' ) && Match( p + 1, s, cs );
      break;

    default:
      result = cs ? ( *s == *p ) : ( ::tolower( *s ) == ::tolower( *p ) );
      result = result && Match( p + 1, s + 1, cs );
  }
  return result;
}

} // namespace

bool
bci::WildcardMatch( const string& inPattern, const string& inString, bool inCaseSensitive )
{
#if BCIDEBUG
  static bool tested = false;
  if( !tested )
  {
    tested = true;
    RunTests();
  }
#endif // BCIDEBUG
  return Match( inPattern.c_str(), inString.c_str(), inCaseSensitive );
}
