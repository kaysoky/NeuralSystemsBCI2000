//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
USERES("calibgen.res");
USEFORM("CalibGenMain.cpp", fMain);
USEFORM("CalibGenHelp.cpp", fHelp);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
        try
        {
                 Application->Initialize();
                 Application->Title = "Calibration File Generator";
                 Application->CreateForm(__classid(TfMain), &fMain);
                 Application->CreateForm(__classid(TfHelp), &fHelp);
                 Application->Run();
        }
        catch (Exception &exception)
        {
                 Application->ShowException(&exception);
        }
        return 0;
}
//---------------------------------------------------------------------------
