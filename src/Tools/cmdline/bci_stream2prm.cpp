////////////////////////////////////////////////////////////////////
// File:    bci_stream2prm.cpp
// Date:    Jan 26, 2005
// Author:  juergen.mellinger@uni-tuebingen.de
// Description: See the ToolInfo definition below.
////////////////////////////////////////////////////////////////////
#include <iostream>
#include <string>

#include "bci_tool.h"
#include "shared/defines.h"
#include "shared/UParameter.h"
#include "shared/MessageHandler.h"

using namespace std;

string ToolInfo[] =
{
  "bci_stream2prm",
  "0.1.0, compiled " __DATE__,
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
  virtual bool HandlePARAM( istream& );
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
StreamToPrm::HandlePARAM( istream& is )
{
  PARAM p;
  if( p.ReadBinary( is ) )
    mrOut << p << "\r\n";
  return true;
}