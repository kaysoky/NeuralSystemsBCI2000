//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A string that may be both quoted, and URL-encoded.
//   For output, quotes are used when necessary.
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

#include "HybridString.h"
#include "EncodedString.h"
#include <sstream>
#include <functional>

using namespace std;

ostream&
HybridString::WriteToStream( ostream& os ) const
{
  return EncodedString( *this ).WriteToStream( os );
}

istream&
HybridString::ReadFromStream( istream& is )
{
  if( ReadUntil( is >> ws, ::isspace ) )
    Decode();
  return is;
};

istream&
HybridString::GetLine( istream& is, char delim )
{
  if( ReadUntil( is, bind2nd( equal_to<int>(), delim ) ) )
  {
    is.ignore();
    Decode();
  }
  return is;
}

void
HybridString::Decode()
{
  istringstream iss( *this );
  EncodedString s;
  iss >> s;
  *this = s;
}
