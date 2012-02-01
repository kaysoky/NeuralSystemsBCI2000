//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: An exception class to report BCI2000 run-time errors,
//   and a std::ostream based object that allows convenient creation
//   of exceptions with message content.
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

#include "BCIException.h"

using namespace std;

BCIException::BCIException( ostream& inStream )
{
  ostringstream* pStream = dynamic_cast<ostringstream*>( &inStream );
  if( pStream )
  {
    pStream->flush();
    mMessage = pStream->str();
  }
  else
  {
    mMessage = "BCIException: No message available (must be constructed from std::ostringstream object)";
  }
}

const char*
BCIException::what() const throw()
{
  return mMessage.data();
}

