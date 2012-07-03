//////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: The Operator module's main() function.
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
///////////////////////////////////////////////////////////////////////
#include <QtGui/QApplication>
#include "Settings.h"
#include "MetaTypes.h"
#include "MainWindow.h"
#include "GenericSignal.h"
#include "BitmapImage.h"
#include "VisDisplay.h"
#include "OSThread.h"
#include "ExceptionCatcher.h"
#include "ProcessUtils.h"
#include "FileUtils.h"
#if _WIN32
# include "FPExceptMask.h"
#endif // _WIN32
#include "../OperatorLib/BCI_OperatorLib.h"

using namespace std;

void MainLoop( QApplication&, MainWindow& );

#ifdef _WIN32
# include <Windows.h>

int main( int, char*[] );

int WINAPI
WinMain( HINSTANCE, HINSTANCE, LPSTR, int )
{
  return main( __argc, __argv );
}
#endif // _WIN32

int
main( int argc, char *argv[] )
{
  if( !ProcessUtils::AssertSingleInstance( argc, argv ) )
    return 0;
#if _WIN32
  FPExceptMask mask;
#endif // _WIN32
  QApplication a( argc, argv );
  a.setOrganizationName( "BCI2000" );
  a.setOrganizationDomain( "bci2000.org" );
  a.setApplicationName( FileUtils::ApplicationTitle().c_str() );
  qRegisterMetaType< GenericSignal >();
  qRegisterMetaType< BitmapImage >();
  Settings::SetFile();
  MainWindow w;
  VisDisplay::SetParentWindow( &w );
  FunctionCall< void( QApplication&, MainWindow& ) >
    call( MainLoop, a, w );
  bool finished = ExceptionCatcher()
    .SetMessage( "Terminating Operator module" )
    .Run( call );
  return finished ? 0 : -1;
}

void
MainLoop( QApplication& a, MainWindow& w )
{
  while( !w.Terminating() )
  { // We use our own event loop to allow for processing pending
    // callbacks from the BCI operator library.
    // Qt docs suggest using a zero ms timer for doing idle processing,
    // but this leads to high CPU usage.
    a.sendPostedEvents();
    a.processEvents();
    OSThread::Sleep( 1 ); // avoid hogging the CPU
    BCI_CheckPendingCallback();
  }
  while( !w.Terminated() )
  {
    OSThread::Sleep( 1 );
    BCI_CheckPendingCallback();
  }
  // QWidget destructors assume a valid application object,
  // so we delete visualization windows while the application
  // object is still in scope.
  VisDisplay::Clear();
}

