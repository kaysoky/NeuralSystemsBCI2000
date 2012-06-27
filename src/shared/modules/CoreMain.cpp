////////////////////////////////////////////////////////////////////////////////
// $Id$
// Description: main() function definition for core modules.
//
// $BEGIN_BCI2000_LICENSE$
//
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2012: BCI2000 team and many external contributors ]
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
#include "PCHIncludes.h"
#pragma hdrstop

#include "CoreModule.h"

#if USE_QT
# include <QApplication>
# include "CoreModuleQT.h"
# include "FileUtils.h"
#endif

#if _WIN32
int main( int, char*[] );

int WINAPI
WinMain( HINSTANCE, HINSTANCE, LPSTR, int )
{
  return main( __argc, __argv );
}
#endif // _WIN32

#if USE_QT
int
main( int argc, char *argv[] )
{
  QApplication::setApplicationName( ( "BCI2000 " + FileUtils::ApplicationTitle() ).c_str() );
  bool success = CoreModuleQT().Run( argc, argv );
  return success ? 0 : -1;
}
#else
int main( int argc, char** argv )
{
  bool success = CoreModule().Run( argc, argv );
  return ( success ? 0 : -1 );
}
#endif

