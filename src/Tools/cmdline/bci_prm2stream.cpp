////////////////////////////////////////////////////////////////////
// File:    bci_prm2stream.cpp
// Date:    Jul 9, 2003
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
  "bci_prm2stream",
  "0.1.0, compiled " __DATE__,
  "Convert a BCI2000 compliant parameter file input into a BCI2000 stream.",
  "Reads a BCI2000 parameter file (*.prm) compliant stream from "
    "standard input and writes it to the standard output as a BCI2000 "
    "compliant binary stream.",
  ""
};

ToolResult ToolInit()
{
  return noError;
}

ToolResult ToolMain( const OptionSet&, istream& in, ostream& out )
{
  ToolResult result = noError;
  PARAM p;
  while( in >> p )
    MessageHandler::PutMessage( out, p );
  return result;
}
