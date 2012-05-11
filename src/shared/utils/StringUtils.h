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
#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <string>

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
} // namespace StringUtils

#endif // STRING_UTILS_H

