////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: The main() function of the BCI2000CheckEnvironment tool.
//    To make sense, this program must link statically to the runtime libraries.
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
#ifndef _WIN32
# error This program is for MS Windows only.
#endif // _WIN32

#include <Windows.h>
#include "MSVCRuntime.h"

const char* gProgramName = "BCI2000 Environment Check";

int WINAPI
WinMain( HINSTANCE, HINSTANCE, LPSTR, int )
{
  bool result = true;
  result &= CheckMSVCRuntime();
  ::MessageBox(
    NULL,
    "The BCI2000 Environment Check has finished.",
    gProgramName,
    MB_OK | MB_ICONINFORMATION
  );
  return ( result ? 0 : 1 );
}
