////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: See the ToolInfo definition below.
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
#include <iostream>
#include <cstdio>
#include <string>

#include "bci_tool.h"
#include "Param.h"
#include "ParamList.h"
#include "MessageChannel.h"
#include "Version.h"

using namespace std;

string ToolInfo[] =
{
  "bci_stream2prm",
  PROJECT_VERSION,
  "Convert a BCI2000 message stream into a BCI2000 parameter file.",
  "Reads a BCI2000 message stream from standard input and extracts "
    "parameter messages to the standard output as a BCI2000 parameter "
    "(.prm) file.",
  "binary",
  ""
};

class StreamToPrm : public MessageChannel
{
 public:
  StreamToPrm( istream& arIn, ostream& arOut )
  : MessageChannel( arIn, arOut ) {}

 private:
  virtual bool OnParam( istream& );
};

ToolResult ToolInit()
{
  return noError;
}

ToolResult ToolMain( OptionSet& arOptions, istream& arIn, ostream& arOut )
{
  if( arOptions.size() > 0 )
    return illegalOption;
  StreamToPrm converter( arIn, arOut );
  while( arIn && arIn.peek() != EOF )
    converter.HandleMessage();
  if( !arIn )
    return illegalInput;
  return noError;
}

bool
StreamToPrm::OnParam( istream& is )
{
  Param p;
  if( p.ReadBinary( is ) )
    Output() << p << "\r\n";
  return true;
}
