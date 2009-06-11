//---------------------------------------------------------------------------

#include <vcl.h>
#include "CoreModuleVCL.h"
#include <float.h>
#pragma hdrstop
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//USEFORM("impedanceGUI.cpp", impGUI);
//---------------------------------------------------------------------------
USEFORM("impedanceGUI.cpp", impGUI);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
        try
        {
				 Application->Initialize();
				 Application->Title = "V-Amp Signal Source";
                 CoreModule().Run( _argc, _argv );
        }
        catch (Exception &exception)
        {
                 Application->ShowException(&exception);
        }
        catch (...)
        {
                 try
                 {
                         throw Exception("");
                 }
                 catch (Exception &exception)
                 {
                         Application->ShowException(&exception);
                 }
        }
        return 0;
}
//---------------------------------------------------------------------------
