////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: griffin.milsap@gmail.com
// Description: A class that implements the CoreModule GUI interface functions
//          for QT-based modules, and an appropriate main() function.
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
#include "PCHIncludes.h"
#pragma hdrstop

#include "CoreModuleQT.h"

bool
CoreModuleQT::Run( int argc, char *argv[])
{
  // Create a QApplication
  QApplication a( argc, argv );
  // Run the BCI2000 Core loop
  return CoreModule::Run( argc, argv );
}

void 
CoreModuleQT::ProcessGUIMessages()
{
  if( qApp )
  {
    qApp->sendPostedEvents();
    qApp->processEvents();
  }
}

bool
CoreModuleQT::GUIMessagesPending()
{
  if( qApp )
    return qApp->hasPendingEvents();
  else
    return false;
}
