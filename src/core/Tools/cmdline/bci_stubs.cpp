////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A file to hold dummy implementations for functions that are
//         unneeded/unwanted when a filter is wrapped into a command line tool.
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "BCIDirectory.h"
#include "FileWriterBase.h"
#include <iostream>
#include <cstdlib>

using namespace std;

// BCIDirectory

BCIDirectory::BCIDirectory()
: mSessionNumber( none ),
  mDesiredRunNumber( none ),
  mActualRunNumber( none ),
  mFileExtension( "" )
{
}

BCIDirectory&
BCIDirectory::UpdateRunNumber()
{
  return *this;
}

string
BCIDirectory::AbsolutePath( const string& inPath )
{
  return inPath;
}
