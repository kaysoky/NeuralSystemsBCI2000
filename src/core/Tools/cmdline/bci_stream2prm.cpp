////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: See the ToolInfo definition below.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////
#include <iostream>
#include <string>

#include "bci_tool.h"
#include "Param.h"
#include "ParamList.h"
#include "MessageHandler.h"
#include "Version.h"

using namespace std;

string ToolInfo[] =
{
  "bci_stream2prm",
  BCI2000_VERSION,
  "Convert a BCI2000 message stream into a BCI2000 parameter file.",
  "Reads a BCI2000 message stream from standard input and extracts "
    "parameter messages to the standard output as a BCI2000 parameter "
    "(.prm) file.",
  ""
};

class StreamToPrm : public MessageHandler
{
 public:
  StreamToPrm( ostream& arOut )
  : mrOut( arOut ) {}

 private:
  ostream& mrOut;
  virtual bool HandleParam( istream& );
};

ToolResult ToolInit()
{
  return noError;
}

ToolResult ToolMain( const OptionSet& arOptions, istream& arIn, ostream& arOut )
{
  if( arOptions.size() > 0 )
    return illegalOption;
  StreamToPrm converter( arOut );
  while( arIn && arIn.peek() != EOF )
    converter.HandleMessage( arIn );
  if( !arIn )
    return illegalInput;
  return noError;
}

bool
StreamToPrm::HandleParam( istream& is )
{
  Param p;
  if( p.ReadBinary( is ) )
    mrOut << p << "\r\n";
  return true;
}
