//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
USEFORM("..\UMain.cpp", fMain);
USEFORM("Usr.cpp", User);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
        try
        {
                 Application->Initialize();
                 Application->Title = "2-dimensional box task";
                 Application->CreateForm(__classid(TfMain), &fMain);
         Application->CreateForm(__classid(TUser), &User);
         Application->Run();
        }
        catch (Exception &exception)
        {
                 Application->ShowException(&exception);
        }
        return 0;
}
//---------------------------------------------------------------------------
