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
#ifndef TINY_STRING_UTILS_H
#define TINY_STRING_UTILS_H

#include <string>
#include <iostream>

namespace Tiny
{

namespace StringUtils
{
  std::wstring ToWide( const char* );
  inline std::wstring ToWide( const std::string& s ) { return ToWide( s.c_str() ); }
  inline std::wstring FromNarrow( const char* s ) { return ToWide( s ); }
  inline std::wstring FromNarrow( const std::string& s ) { return ToWide( s.c_str() ); }

  std::string ToNarrow( const wchar_t* );
  inline std::string ToNarrow( const std::wstring& s ) { return ToNarrow( s.c_str() ); }
  inline std::string FromWide( const wchar_t* s ) { return ToNarrow( s ); }
  inline std::string FromWide( const std::wstring& s ) { return ToNarrow( s.c_str() ); }

  extern const std::string WhiteSpace;
  std::string LStrip( const std::string&, const std::string& = WhiteSpace );
  std::string RStrip( const std::string&, const std::string& = WhiteSpace );
  std::string Strip( const std::string&, const std::string& = WhiteSpace );

  std::wstring ToUpper( const std::wstring& );
  std::wstring ToLower( const std::wstring& );
  inline std::string ToUpper( const std::string& s ) { return ToNarrow( ToUpper( ToWide( s ) ) ); }
  inline std::string ToLower( const std::string& s ) { return ToNarrow( ToLower( ToWide( s ) ) ); }

  bool CiLess( const std::wstring&, const std::wstring& );
  bool CiLess( const std::string&, const std::string& );
  template<class T> struct CiRef
  {
    const T& a;
    operator const T&() { return a; }
    bool operator==( const T& b ) const { return !CiLess( a, b ) && !CiLess( b, a ); }
    bool operator!=( const T& b ) const { return CiLess( a, b ) || CiLess( b, a ); }
    bool operator<( const T& b ) const { return CiLess( a, b ); }
    bool operator>=( const T& b ) const { return !CiLess( a, b ); }
    bool operator>( const T& b ) const { return CiLess( b, a ); }
    bool operator<=( const T& b ) const { return !CiLess( b, a ); }
  };
  template<class T> CiRef<T> Ci( const T& t )
  { CiRef<T> r = { t }; return r; }

  std::ostream& WriteAsBase64( std::ostream&, const std::string& );
  std::istream& ReadAsBase64( std::istream&, std::string&, int stopAtChar );
  std::istream& ReadAsBase64( std::istream&, std::string&, int (*stopIf)( int ) = 0 );
} // namespace StringUtils

} // namespace

template<class T>
bool operator==( const T& a, Tiny::StringUtils::CiRef<T> b )
{ return b == a; }
template<class T>
bool operator!=( const T& a, Tiny::StringUtils::CiRef<T> b )
{ return b != a; }
template<class T>
bool operator<( const T& a, Tiny::StringUtils::CiRef<T> b )
{ return b >= a; }
template<class T>
bool operator<=( const T& a, Tiny::StringUtils::CiRef<T> b )
{ return b > a; }
template<class T>
bool operator>( const T& a, Tiny::StringUtils::CiRef<T> b )
{ return b <= a; }
template<class T>
bool operator>=( const T& a, Tiny::StringUtils::CiRef<T> b )
{ return b < a; }

namespace StringUtils = Tiny::StringUtils;
using StringUtils::Ci;


#endif // TINY_STRING_UTILS_H

