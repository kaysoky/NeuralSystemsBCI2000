/*
BCI2000Certification.cpp
Author: Adam Wilson, University of Wisconsin-Madison, 2007

$BEGIN_BCI2000_LICENSE$

This file is part of BCI2000, a platform for real-time bio-signal research.
[ Copyright (C) 2000-2012: BCI2000 team and many external contributors ]

BCI2000 is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

BCI2000 is distributed in the hope that it will be useful, but
                        WITHOUT ANY WARRANTY
- without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program.  If not, see <http://www.gnu.org/licenses/>.

$END_BCI2000_LICENSE$
*/
//---------------------------------------------------------------------------
#include <stdio.h>
#include <string>
#include <vector>
#include <algorithm>
#include <QApplication>

#include "analysisGUI.h"
#include "ExceptionCatcher.h"

using namespace std;

#ifdef _WIN32
# include <Windows.h>

int main( int, char*[] );

int WINAPI
WinMain( HINSTANCE, HINSTANCE, LPSTR, int )
{
  return main( __argc, __argv );
}
#endif // _WIN32

int main(int argc, char* argv[])
{
  QApplication app(argc, argv);
  analysisGUI GUI;
  GUI.show();

  std::string message = "Aborting ";
  message += app.applicationName().toLocal8Bit().constData();
  FunctionCall< int() >
    call( &QApplication::exec );
  bool finished = ExceptionCatcher()
    .SetMessage( message )
    .Run( call );
  return finished ? call.Result() : -1;
}
//---------------------------------------------------------------------------
