//---------------------------------------------------------------------------
#include "PCHIncludes.h"
#pragma hdrstop
#include <vcl.h>
USEFORM("..\..\shared\UVisConfig.cpp", fVisConfig);
USEFORM("..\UMain.cpp", fMain);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
        try
        {
                 Application->Initialize();
                 Application->Title = "P3SignalProcessing";
                 Application->CreateForm(__classid(TfMain), &fMain);
                 Application->CreateForm(__classid(TfVisConfig), &fVisConfig);
                 Application->Run();
        }
        catch (Exception &exception)
        {
                 Application->ShowException(&exception);
        }
        return 0;
}
//---------------------------------------------------------------------------
