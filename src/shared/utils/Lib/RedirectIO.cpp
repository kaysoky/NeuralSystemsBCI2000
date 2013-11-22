//////////////////////////////////////////////////////////////////////
// $Id: Debugging.h 4606 2013-10-11 16:14:35Z mellinger $
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Standard iostream redirection.
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
#include "RedirectIO.h"
#include "StaticObject.h"

using namespace std;

namespace
{

streambuf
  *pbuf_cin,
  *pbuf_cout,
  *pbuf_cerr,
  *pbuf_clog;

struct IStream : istream { IStream() : istream( 0 ) {} };
struct OStream : ostream { OStream() : ostream( 0 ) {} };
StaticObject<IStream> C_in;
StaticObject<OStream> C_out, C_err, C_log;

void CheckInit()
{
  if( !pbuf_cin )
    pbuf_cin = cin.rdbuf();
  if( !pbuf_cout )
  {
    pbuf_cout = cout.rdbuf();
    pbuf_cerr = cerr.rdbuf();
    pbuf_clog = clog.rdbuf();
  }
}

}

namespace Tiny
{

#define STREAM(x) \
C##x()\
{\
  if( !C_##x->rdbuf() )\
  {\
    CheckInit();\
    C_##x->rdbuf( pbuf_c##x );\
  }\
  return *C_##x;\
}

istream& STREAM(in)
ostream& STREAM(out)
ostream& STREAM(err)
ostream& STREAM(log)

void
Redirect( istream& from, istream& to )
{
  CheckInit();
  from.rdbuf( to.rdbuf() );
}

void
Redirect( ostream& from, ostream& to )
{
  CheckInit();
  from.rdbuf( to.rdbuf() );
}

} // namespace

