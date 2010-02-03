////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class that implements the CoreModule GUI interface functions
//          for VCL-based modules.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef CORE_MODULE_VCL_H
#define CORE_MODULE_VCL_H

#include "CoreModule.h"
#include <vcl.h>
#include <windows.h>

class CoreModuleVCL : public CoreModule
{
  virtual void ProcessGUIMessages()
  {
    Application->ProcessMessages();
    ::Sleep( 0 );
  }
  virtual bool GUIMessagesPending()
  {
    return ::GetQueueStatus( QS_ALLINPUT );
  }
};

#endif // CORE_MODULE_VCL_H
