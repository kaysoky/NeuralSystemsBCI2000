////////////////////////////////////////////////////////////////////
// File:    bci_filtertool.cpp
// Date:    Jul 18, 2003
// Author:  juergen.mellinger@uni-tuebingen.de
// Description: A BCI2000 filter wrapper that reads a BCI2000
//          compliant binary stream from standard input, applies
//          a BCI2000 filter, and writes its output to the
//          standard output as a BCI2000 compliant binary stream.
////////////////////////////////////////////////////////////////////
#include "bci_tool.h"

#include "bci_filterwrapper.h"
#include <fstream>

#define FILTER_NAME "$FILTER$"

using namespace std;

string ToolInfo[] =
{
  "",
  "tool, framework version 0.1.0, compiled "__DATE__,
  "Process standard input with the \"" FILTER_NAME "\" BCI2000 filter.",
  "Reads a BCI2000 compliant binary stream from standard input, applies the\n"
    FILTER_NAME " BCI2000 filter, and writes its output to standard output\n"
    "as a BCI2000 compliant binary stream.",
  "-o<file>, --operator<file>\tdirect visualization messages to <file>",
  "                          \tinstead of /dev/null",
  ""
};


ToolResult
ToolInit()
{
  const char* pFilterName = FilterWrapper::FilterName();
  if( pFilterName == NULL )
    return genericError;
  for( int i = 0; ToolInfo[ i ] != ""; ++i )
  {
    size_t namePos;
    while( ( namePos = ToolInfo[ i ].find( FILTER_NAME ) ) != string::npos )
      ToolInfo[ i ].replace( namePos, string( FILTER_NAME ).length(), pFilterName );
  }
  return noError;
}

ToolResult
ToolMain( const OptionSet& arOptions, istream& arIn, ostream& arOut )
{
  ofstream operatorStream;
  FilterWrapper wrapper( arOut, operatorStream );
  if( arOptions.size() == 1 )
  {
    string operatorFile = arOptions.getopt( "-o|-O|--operator", "" );
    if( operatorFile == "" )
      return illegalOption;
    operatorStream.open( operatorFile.c_str() );
  }
  while( arIn && arIn.peek() != EOF )
    wrapper.HandleMessage( arIn );
  wrapper.FinishProcessing();
  if( !arIn )
    return illegalInput;
  return noError;
}
