//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
USEFORM("Usr.cpp", User);
USEFORM("..\..\shared\UCoreMain.cpp", fMain);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
        try
        {
                 Application->Initialize();
                 Application->Title = "3-dimensional box task";
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
