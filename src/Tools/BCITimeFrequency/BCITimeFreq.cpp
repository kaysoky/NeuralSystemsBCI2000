//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
USEFORM("UMain.cpp", fMain);
USEFORM("StateForm1.cpp", UseStateForm);
USEFORM("OutputForm1.cpp", OutputForm);
USEFORM("InputForm1.cpp", InputForm);
USEFORM("ProcessForm1.cpp", ProcessForm);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
        try
        {
                 Application->Initialize();
                 Application->Title = "BCI2000toGAB";
                 Application->CreateForm(__classid(TfMain), &fMain);
                 Application->CreateForm(__classid(TUseStateForm), &UseStateForm);
                 Application->CreateForm(__classid(TOutputForm), &OutputForm);
                 Application->CreateForm(__classid(TInputForm), &InputForm);
                 Application->CreateForm(__classid(TProcessForm), &ProcessForm);
                 Application->Run();
        }
        catch (Exception &exception)
        {
                 Application->ShowException(&exception);
        }
        return 0;
}
//---------------------------------------------------------------------------
