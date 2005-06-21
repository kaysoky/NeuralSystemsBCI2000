#include "PCHIncludes.h"
#pragma hdrstop
#include <vcl.h>
//---------------------------------------------------------------------------
USEFORM("..\..\shared\UCoreMain.cpp", fMain);
USEFORM("Unit1.cpp", fRPcoX);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
        try
        {
                 Application->Initialize();
                 Application->Title = "EEGsource V0.25";
                 Application->CreateForm(__classid(TfMain), &fMain);
         Application->CreateForm(__classid(TfRPcoX), &fRPcoX);
         Application->Run();
        }
        catch (Exception &exception)
        {
                 Application->ShowException(&exception);
        }
        return 0;
}
