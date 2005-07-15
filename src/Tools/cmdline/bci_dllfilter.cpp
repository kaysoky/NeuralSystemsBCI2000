////////////////////////////////////////////////////////////////////
// File:    bci_dllfilter.cpp
// Date:    Jul 18, 2003
// Author:  juergen.mellinger@uni-tuebingen.de
// Description: Reads a BCI2000 compliant binary stream from
//          standard input, applies a BCI2000 filter wrapped inside
//          a DLL, and writes its output to the
//          standard output as a BCI2000 compliant binary stream.
////////////////////////////////////////////////////////////////////
#include "bci_tool.h"

#include "bci_dll.h"
#include "LengthField.h"
#include <fstream>
#include <sstream>
#include <windows.h>

using namespace std;

string ToolInfo[] =
{
  "",
  "tool, framework version 0.1.0, compiled " __DATE__,
  "Process standard input with a BCI2000 filter wrapped into a DLL.",
  "Reads a BCI2000 compliant binary stream from standard input, applies the\n"
    "BCI2000 filter contained in the named dll, and writes its output to\n"
    "standard output as a BCI2000 compliant binary stream.",
  "-o<file>, --operator<file>\tdirect visualization messages to <file>",
  "                          \tinstead of /dev/null",
  "-f<name>, --filter<name>  \tapply a filter specified by the name",
  "                          \tof its dll file",
  ""
};

ToolResult
ToolInit()
{
  return noError;
}

ToolResult
ToolMain( const OptionSet& arOptions, istream& arIn, ostream& arOut )
{
  string dllName = arOptions.getopt( "-f|-F|--filter", "" );
  if( dllName == "" )
  {
    cerr << "You must specify the name of a filter dll" << endl;
    return illegalOption;
  }
  HINSTANCE dllHandle = ::LoadLibrary( dllName.c_str() );
  GetInfoPtr GetInfo
    = ( GetInfoPtr )::GetProcAddress( dllHandle, "GetInfo" );
  ProcessStreamDataPtr ProcessStreamData
    = ( ProcessStreamDataPtr )::GetProcAddress( dllHandle, "ProcessStreamData" );
  FinishProcessingPtr FinishProcessing
    = ( FinishProcessingPtr )::GetProcAddress( dllHandle, "FinishProcessing" );
  if( !( dllHandle && GetInfo && ProcessStreamData && FinishProcessing ) )
  {
    cerr << "Could not load library " << dllName << endl;
    return fileIOError;
  }
  ofstream visStream;
  string visFile = arOptions.getopt( "-o|-O|--operator", "" );
  visStream.open( visFile.c_str() );
  if( !visFile.empty() && !visStream.is_open() )
  {
    cerr << "Could not open " << visFile << " for output" << endl;
    return fileIOError;
  }

  BCIDLL_bufferDesc input, output, vis, error;
  input.data = NULL;
  long  maxInputLength = 0;
  bool success = true;
  while( arIn && arIn.peek() != EOF && success )
  {
    int desc = arIn.get(),
        supp = arIn.get();
    LengthField<2> messageLength;
    messageLength.ReadBinary( arIn );
    if( arIn )
    {
      stringstream lengthFieldBuf;
      messageLength.WriteBinary( lengthFieldBuf );
      int headerLength = lengthFieldBuf.str().length() + 2;
      input.length = headerLength + messageLength;
      if( maxInputLength < input.length )
      {
        delete [] input.data;
        input.data = new char[ input.length ];
        maxInputLength = input.length;
      }
      input.data[ 0 ] = desc;
      input.data[ 1 ] = supp;
      lengthFieldBuf.read( input.data + 2, lengthFieldBuf.str().length() );
      success = arIn.read( input.data + headerLength, messageLength );
    }
    success = success && ProcessStreamData( &input, &output, &vis, &error );
    cout.write( output.data, output.length );
    visStream.write( vis.data, vis.length );
    cerr.write( error.data, error.length );
  }
  delete [] input.data;
  success = success && FinishProcessing( &output, &vis, &error );
  cout.write( output.data, output.length );
  visStream.write( vis.data, vis.length );
  cerr.write( error.data, error.length );
  if( !arIn )
    return illegalInput;
  return noError;
}
