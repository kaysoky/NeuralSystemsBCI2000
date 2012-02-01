////////////////////////////////////////////////////////////////////
// $Id$
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: A function that hides compiler specific details
//              involved in translating the result of a
//              typeinfo::name() call into a human readable class
//              name.
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
////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "ClassName.h"

#ifdef __GNUC__
# include <cxxabi.h>
# include <cstdlib>
#endif

using namespace std;
using namespace bci;

string
bci::ClassName( const type_info& inTypeid )
{
  string result = inTypeid.name();

#ifdef __GNUC__
  int err = 0;
  char* name = abi::__cxa_demangle( result.c_str(), 0, 0, &err );
  if( name )
  {
    result = name;
    ::free( name );
  }
#endif // __GNUC__

  // To obtain a useful representation of the class name, we need to do some
  // processing that removes white space and compiler specific additions.

  // First, remove white space between template arguments and <> brackets.
  static struct
  {
    const char* original,
              * replacement;
  } replacementTable[] =
  {
    { ", ", "," },
    { "< ", "<" },
    { " >", ">" },
    { "class ", "" },
    { "struct ", "" },
    { "enum ", "" },
  };
  size_t pos;
  for( size_t r = 0; r < sizeof( replacementTable ) / sizeof( *replacementTable ); ++r )
  {
    string original = replacementTable[r].original;
    while( string::npos != ( pos = result.find( original ) ) )
      result = result.replace( pos, original.length(), replacementTable[r].replacement );
  }
  return result;
}

