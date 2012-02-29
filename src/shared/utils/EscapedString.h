//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A string class that handles C-style backslash
//   escaping, and quoting.
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
#ifndef ESCAPED_STRING_H
#define ESCAPED_STRING_H

#include <string>
#include <iostream>

struct EscapedString : std::string
{
  EscapedString() {}
  EscapedString( const std::string& s ) : std::string( s ) {}
  EscapedString( const char* s ) : std::string( s ) {}

  std::ostream& WriteToStream( std::ostream& ) const;
  std::istream& ReadFromStream( std::istream& );
};

inline
std::ostream& operator<<( std::ostream& s, const EscapedString& e )
{
  return e.WriteToStream( s );
}

inline
std::istream& operator>>( std::istream& s, EscapedString& e )
{
  return e.ReadFromStream( s );
}

#endif // ESCAPED_STRING_H
