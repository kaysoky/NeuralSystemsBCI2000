////////////////////////////////////////////////////////////////////
// File:    bci_prm2stream.cpp
// Date:    Jul 9, 2003
// Author:  juergen.mellinger@uni-tuebingen.de
// Description: See the ToolInfo definition below.
////////////////////////////////////////////////////////////////////
#include <iostream>
#include <string>

#include "bci2000_types.h"
#include "bci_tool.h"

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

  string token;
  bool legalInput = true;
  while( legalInput && in.peek() != EOF && getline( in, token ) )
  {
    int length = token.length() - 1;
    legalInput &= ( length < ( 1 << 16 ) );
    char parameterHeader[] =
    {
      parameter,
      none,
      length & 0xff,
      ( length >> 8 ) & 0xff
    };
    out.write( parameterHeader, sizeof( parameterHeader ) );
    out.write( token.c_str(), length );
  }

  return result;
}
