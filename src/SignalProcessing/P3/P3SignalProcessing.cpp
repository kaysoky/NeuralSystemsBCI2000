//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
USEFORM("..\..\shared\UVisConfig.cpp", fVisConfig);
USEFORM("..\UMain.cpp", fMain);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
        try
        {
                 Application->Initialize();
                 Application->Title = "MXSignalProcessing";
                 Application->CreateForm(__classid(TfVisConfig), &fVisConfig);
                 Application->CreateForm(__classid(TfMain), &fMain);
                 Application->Run();
        }
        catch (Exception &exception)
        {
                 Application->ShowException(&exception);
        }
        return 0;
}
//---------------------------------------------------------------------------
