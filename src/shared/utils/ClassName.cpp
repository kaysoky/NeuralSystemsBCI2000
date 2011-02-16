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
// [ Copyright (C) 2000-2011: BCI2000 team and many external contributors ]
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
#include <ctype.h>

using namespace bci;

const char*
bci::ClassName( const std::type_info& inTypeid )
{
  const char* result = inTypeid.name();
#ifdef __GNUC__
  static std::string nameBuffer;
  int err = 0;
  char* name = abi::__cxa_demangle( result, 0, 0, &err );
  if( name )
  {
    nameBuffer = name;
    free( name );
  }
  else
  {
    nameBuffer = "<N/A>";
  }
  result = nameBuffer.c_str();
#endif
  const char* p = result;
  while( *p != '\0' )
    if( ::isspace( *p++ ) )
      result = p;
  return result;
}

