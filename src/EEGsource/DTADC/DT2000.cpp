//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
USEFORM("..\UMain.cpp", fMain);
USEFORM("..\..\shared\UVisConfig.cpp", fVisConfig);
USEFORM("MsgWin1.cpp", MsgWin);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
        try
        {
                 Application->Initialize();
                 Application->Title = "EEGsource V0.20";
                 Application->CreateForm(__classid(TfMain), &fMain);
                 Application->CreateForm(__classid(TfVisConfig), &fVisConfig);
                 Application->CreateForm(__classid(TMsgWin), &MsgWin);
                 Application->Run();
        }
        catch (Exception &exception)
        {
                 Application->ShowException(&exception);
        }
        return 0;
}
//---------------------------------------------------------------------------
