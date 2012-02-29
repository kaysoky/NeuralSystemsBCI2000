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
#include "PCHIncludes.h"
#pragma hdrstop

#include "EscapedString.h"
#include <iomanip>
#include <sstream>

using namespace std;

ostream&
EscapedString::WriteToStream( ostream& os ) const
{
  const struct { char in; const char* out; }
  replacements[] =
  {
    { ' ', "\\ " },
    { '\t', "\\t" },
    { '\n', "\\n" },
    { '\r', "\\r" },
    { '\"', "\\\"" },
    { '\'', "\\'" },
  };
  const size_t count = sizeof( replacements ) / sizeof( *replacements );
  for( const_iterator i = begin(); i != end(); ++i )
  {
    size_t j = 0;
    while( j < count && *i != replacements[j].in )
      ++j;
    if( j < count )
      os << replacements[j].out;
    else if( ::isprint( *i ) )
      os.put( *i );
    else
      os << "\\x" << setw( 2 ) << setfill( '0' ) << hex << *i;
  }
  return os;
}

istream&
EscapedString::ReadFromStream( istream& is )
{
  const struct { char code; char ch; }
  escapeCodes[] =
  {
    { 't', '\t' },
    { 'n', '\n' },
    { 'r', '\r' },
  };
  const size_t count = sizeof( escapeCodes ) / sizeof( *escapeCodes );

  clear();
  bool done = false,
       withinQuotes = false;
  while( !done )
  {
    int c = is.peek();
    switch( c )
    {
      case EOF:
        done = true;
        break;
        
      case '\\':
      {
        is.get();
        int next = is.peek();
        if( next == EOF )
        {
          done = true;
          ( *this ) += '\\';
        }
        else
        {
          is.get();
          size_t i = 0;
          while( i < count && next != escapeCodes[i].code )
            ++i;
          if( i < count )
            ( *this ) += escapeCodes[i].ch;
          else if( next == 'x' )
          {
            if( ::isxdigit( is.peek() ) )
            {
              istringstream iss;
              iss.str() += is.get();
              if( ::isxdigit( is.peek() ) )
                iss.str() += is.get();
              int ch;
              iss >> hex >> ch;
              ( *this ) += ch;
            }
            else
              ( *this ) += "\\x";
          }
          else
            ( *this ) += next;
        }
      } break;

      case '\"':
        withinQuotes = !withinQuotes;
        break;

      default:
        if( ::isspace( c ) && !withinQuotes )
          done = true;
        else
          ( *this ) += is.get();
    }
  }
  return is;
}
