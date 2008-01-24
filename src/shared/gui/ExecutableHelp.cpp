////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class that displays an executable's associated
//   help file.
//   The help file is a html file that has the same name as the executable,
//   except that it bears a .html extension.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "ExecutableHelp.h"
#include <string>

using namespace std;

static const string sHelpExtension = ".html";

void
ExecutableHelp::Display() const
{
#ifdef _WIN32
  const int increment = 512;
  char* buf = NULL;
  int bufSize = 0,
      bytesCopied = bufSize;
  while( bytesCopied == bufSize )
  {
    bufSize += increment;
    delete[] buf;
    buf = new char[ bufSize ];
    bytesCopied = ::GetModuleFileName( NULL, buf, bufSize );
  }
  string helpFile = buf;
  delete[] buf;

  size_t pos = helpFile.find_last_of( ".\\//" );
  if( pos != string::npos && helpFile[ pos ] != '.' )
    pos = string::npos;
  helpFile = helpFile.substr( 0, pos ) + sHelpExtension;
  pos = helpFile.find_last_of( "\\/" );
  string helpFileDir = helpFile.substr( 0, pos );
  HINSTANCE err = ::ShellExecute( NULL, NULL, helpFile.c_str(), NULL, helpFileDir.c_str(), 0 );
  if( reinterpret_cast<int>( err ) <= 32 )
  {
    ::MessageBox(
      NULL,
      "Help files should be located in the\n"
      "executable's directory.\n\n"
      "Help files bear the executable's name,\n"
      "and a .html extension.",
      "Error Opening Help File",
      MB_OK | MB_ICONERROR
    );
  }
#else // TODO
# error Globally store the executable's path at startup, \
        make BCIDirectory use it rather than GetCWD().
#endif // TODO
}

