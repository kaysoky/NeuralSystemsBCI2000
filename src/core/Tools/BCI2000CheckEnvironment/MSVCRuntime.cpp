////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: This function checks for the presence of MSVC runtime
//   libraries.
//
// $BEGIN_BCI2000_LICENSE$
//
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2011: BCI2000 team and many external contributors ]
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
////////////////////////////////////////////////////////////////////////////////
#include "Globals.h"
#include "MSVCRuntime.h"
#include <Windows.h>
#include <Shellapi.h>

#define LIB_NAME "MSVC Runtime Libraries 9.0"
#define DOWNLOAD_LINK "http://www.bci2000.org/downloads/bin/vcredist_x86.exe"

bool CheckMSVCRuntime()
{
  const char* libraries[] =
  {
    "msvcr90",
    "msvcp90",
  };
  bool ok = true;
  for( int i = 0; i < sizeof( libraries ) / sizeof( *libraries ); ++i )
    // MSVC runtime libraries may not be loaded as DLLs using LoadLibrary
    ok &= ( NULL != ::LoadLibraryEx( libraries[i], NULL, LOAD_LIBRARY_AS_DATAFILE ) );
  if( ok )
  {
    ::MessageBox( 
        NULL, 
        LIB_NAME " are installed.", 
        gProgramName, 
        MB_OK | MB_ICONINFORMATION 
    );
  }
  else
  {
    int result = ::MessageBox( NULL,
                    LIB_NAME " not found.\n\n"
                    "To download the installer, click \"Yes\".\n"
                    "To continue without downloading, click \"No\".",
                    gProgramName,
                    MB_YESNO | MB_ICONWARNING
                  );
    if( IDYES == result )
    {
      HINSTANCE result = ::ShellExecute( NULL,
                      "open",
                      DOWNLOAD_LINK,
                      NULL,
                      NULL,
                      SW_SHOW 
                     );
      if( 32 >= reinterpret_cast<int>( result ) )
        ::MessageBox( 
          NULL, 
          "Could not download " LIB_NAME ".",
          gProgramName,
          MB_OK | MB_ICONWARNING 
        );
    }
  }
  return ok;
}
