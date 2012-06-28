////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class that implements the CoreModule GUI interface functions
//          for VCL-based modules.
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
#ifndef CORE_MODULE_VCL_H
#define CORE_MODULE_VCL_H

#include "CoreModule.h"
#include <vcl.h>
#include <windows.h>

class CoreModuleVCL : public CoreModule
{
  virtual void OnInitialize( int&, char** )
  {
#if __BORLANDC__ >= 0x590 // C++ Builder 2007
    Application->MainFormOnTaskBar = true;
#endif // __BORLANDC__
  }
  virtual void OnProcessGUIMessages()
  {
    Application->ProcessMessages();
    ::Sleep( 0 );
  }
  virtual bool OnGUIMessagesPending()
  {
    return ::GetQueueStatus( QS_ALLINPUT );
  }
};

#endif // CORE_MODULE_VCL_H
