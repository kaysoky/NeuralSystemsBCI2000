/* $BEGIN_BCI2000_LICENSE$
 *
 * This file is part of BCI2000, a platform for real-time bio-signal research.
 * [ Copyright (C) 2000-2012: BCI2000 team and many external contributors ]
 *
 * BCI2000 is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * BCI2000 is distributed in the hope that it will be useful, but
 *                         WITHOUT ANY WARRANTY
 * - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $END_BCI2000_LICENSE$
 */
#include "PCHIncludes.h"

#ifdef __BORLANDC__

#include <vcl.h>
#pragma hdrstop
USEFORM("UDemoWindow.cpp", DemoWindow);

#else // __BORLANDC__

// Not using borland, we'll use Qt instead
#pragma hdrstop
#include <QApplication>
#include "DemoWindow.h"

#endif // __BORLANDC__

#define APP_TITLE "3D API Demo"

//---------------------------------------------------------------------------
#ifdef _WIN32

int main( int argc, char** argv );

int WINAPI
WinMain( HINSTANCE, HINSTANCE, LPSTR, int )
{
#ifdef __BORLANDC__
        try
        {
                 Application->Initialize();
                 Application->Title = APP_TITLE;
                 Application->CreateForm(__classid(TDemoWindow), &DemoWindow);
                 Application->Run();
        }
        catch (Exception &exception)
        {
                 Application->ShowException(&exception);
        }
        return 0;

#else // __BORLANDC__

  return main( __argc, __argv );

#endif // __BORLANDC__
}
#endif // _WIN32

#ifndef __BORLANDC__

int main( int argc, char *argv[] )
{
  QApplication::setApplicationName( APP_TITLE );
  QApplication a( argc, argv );
  DemoWindow w;
  w.show();
  return a.exec();
}

#endif // __BORLANDC__
//---------------------------------------------------------------------------
