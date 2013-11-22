//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Classes for convenient throwing and catching of
//   expected and unexpected exceptions.
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
#include "Exception.h"
#include "Debugging.h"
#include "ClassName.h"

using namespace std;
using namespace Tiny;

string
Exception::ToString( ostream& os )
{
  ostringstream* p = dynamic_cast<ostringstream*>( &os.flush() );
  return p ? p->str() : "bci::Exception::ToString(): Expected std::ostringstream argument";
}

Exception::Exception( const string& inWhat, const string& inWhere, const type_info& inType )
: mWhat( inWhat ), mWhere( inWhere ), mAlreadyShown( false )
{
#if TINY_DEBUG
  bool doDebug = true;
  static const type_info* dontDebug[] =
  {
    &typeid( Tiny::Exception ),
    &typeid( std::exception ),
  };
  for( size_t i = 0; i < sizeof( dontDebug ) / sizeof( *dontDebug ); ++i )
    doDebug = doDebug && ( &inType != dontDebug[i] );
  if( doDebug )
    SuggestDebugging_( "Exception of type " + ClassName( inType ), inWhat + "\n\n" + inWhere );
  mAlreadyShown = doDebug;
#endif // TINY_DEBUG
}
