#include <vcl.h>
#pragma hdrstop

#include <float.h>
#include "UMain.h"
USEFORM("UMain.cpp", fMain);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
        try
        {
                 _control87(MCW_EM,MCW_EM);             // apparently required by Matlab
                 Application->Initialize();
                 Application->Title = "BCI2000toASCII";
                 Application->CreateForm(__classid(TfMain), &fMain);
                 fMain->ProcessCommandLineOptions();
                 Application->Run();
        }
        catch (Exception &exception)
        {
                 Application->ShowException(&exception);
        }
        return 0;
}
//---------------------------------------------------------------------------

