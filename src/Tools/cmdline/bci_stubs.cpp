////////////////////////////////////////////////////////////////////////////////
// File:   bci_stubs.cpp
// Date:   Jan 26, 2005
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A file to hold dummy implementations for functions that are
//         unneeded/unwanted when a filter is wrapped into a command line tool.
////////////////////////////////////////////////////////////////////////////////

#include "shared/BCIDirectry.h"
#include "stdlib.h"

int
BCIDirectory::ProcPath()
{
  return 0;
}

const char*
BCIDirectory::ProcSubDir()
{
  const char* tempDir = ::getenv( "TEMP" );
  if( tempDir == NULL )
    tempDir = "bci_tool_output";
  return tempDir;
}

