//   $Id: PythonSource.cpp 3326 2011-06-17 23:56:38Z jhill $
//  
//   This file is part of the BCPy2000 foundation, a set of modules for
//   the BCI2000 <http://bci2000.org/> that allow communication with a
//   Python framework built on top. It is distributed together with the
//   BCPy2000 framework.
// 
//   Copyright (C) 2007-11  Jeremy Hill, Thomas Schreiner, 
//                         Christian Puzicha, Jason Farquhar
//   
//   bcpy2000@bci2000.org
//   
//   The BCPy2000 foundation is free software: you can redistribute it
//   and/or modify it under the terms of the GNU Lesser General Public
//   License as published by the Free Software Foundation, either
//   version 3 of the License, or (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU Lesser General Public License for more details.
//
//   You should have received a copy of the GNU Lesser General Public License
//   along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#ifndef __BORLANDC__

#include "CoreModule.h"
int main( int argc, char** argv )
{
  bool success = CoreModule().Run( argc, argv );
  return ( success ? 0 : -1 );
}

#if _WIN32
int WINAPI WinMain( HINSTANCE, HINSTANCE, LPSTR, int )
{
  return main( __argc, __argv );
}
#endif // _WIN32

#else // !__BORLANDC__

#include "PCHIncludes.h"
#pragma hdrstop

#include <vcl.h>
#include "CoreModuleVCL.h"

//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
        try
        {
                 Application->Initialize();
                 Application->Title = "BCI2000 PythonSource";\
                 CoreModuleVCL().Run( _argc, _argv );
        }
        catch (Exception &exception)
        {
                 Application->ShowException(&exception);
        }
        return 0;
}
//---------------------------------------------------------------------------

#endif // !__BORLANDC__

