//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
USEFORM("UMain.cpp", fMain);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
        try
        {
                 Application->Initialize();
                 Application->Title = "BCI2000toGAB";
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
