//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
USERES("BCI2GAB.res");
USEFORM("UMain.cpp", fMain);
USEUNIT("..\..\shared\V0.16\UBCI2000Data.cpp");
USEUNIT("..\..\shared\V0.16\UParameter.cpp");
USEUNIT("..\..\shared\V0.16\UState.cpp");
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
