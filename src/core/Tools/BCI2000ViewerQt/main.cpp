////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: The main() function of the BCI2000Viewer tool.
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
#include <QtGui/QApplication>
#include "ExceptionCatcher.h"
#include "Settings.h"
#include "bci2000viewer.h"

#ifdef _WIN32
# include <Windows.h>

int main( int, char*[] );

int WINAPI
WinMain( HINSTANCE, HINSTANCE, LPSTR, int )
{
  return main( __argc, __argv );
}
#endif // _WIN32


int main(int argc, char *argv[])
{
  QApplication a( argc, argv );
  a.setOrganizationName( "BCI2000" );
  a.setOrganizationDomain( "bci2000.org" );
  a.setApplicationName( "BCI2000Viewer" );
  Settings::SetFile();
  BCI2000Viewer w;
  w.show();

  std::string message = "aborting ";
  message += a.applicationName().toLocal8Bit().constData();
  struct
  {
    QApplication& app;
    int result;
    void operator()()
    { result = app.exec(); }
  } functor = { a, -1 };
  ExceptionCatcher().SetMessage( message )
                    .Execute( functor );
  return functor.result;
}
