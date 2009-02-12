/* (C) 2000-2009, BCI2000 Project
/* http://www.bci2000.org
/*/
#include "PCHIncludes.h"
#pragma hdrstop
#include <vcl.h>
#include "CoreModuleVCL.h"
#include <float.h>
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    _control87(MCW_EM,MCW_EM);

        try
        {
                 Application->Initialize();
                 Application->Title = "gUSBamp Source Module";
                 CoreModuleVCL().Run( _argc, _argv );
        }
        catch (Exception &exception)
        {
                 Application->ShowException(&exception);
        }
        return 0;
}
//---------------------------------------------------------------------------

