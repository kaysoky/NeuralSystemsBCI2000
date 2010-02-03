/* (C) 2000-2010, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
//---------------------------------------------------------------------------
USEFORM("UMain.cpp", fMain);
USEFORM("..\..\Operator\UOperatorCfg.cpp", fConfig);
USEFORM("..\..\Operator\UPreferences.cpp", fPreferences);
USEFORM("..\..\Operator\UEditMatrix.cpp", fEditMatrix);
USEFORM("..\..\Operator\UShowParameters.cpp", fShowParameters);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
        try
        {
                 Application->Initialize();
                 Application->Title = "BCI2000 FileInfo";
         Application->CreateForm(__classid(TfMain), &fMain);
         Application->CreateForm(__classid(TfConfig), &fConfig);
         Application->CreateForm(__classid(TfPreferences), &fPreferences);
         Application->CreateForm(__classid(TfEditMatrix), &fEditMatrix);
         Application->CreateForm(__classid(TfShowParameters), &fShowParameters);
         Application->Run();
        }
        catch (Exception &exception)
        {
                 Application->ShowException(&exception);
        }
        catch (...)
        {
                 try
                 {
                         throw Exception("");
                 }
                 catch (Exception &exception)
                 {
                         Application->ShowException(&exception);
                 }
        }
        return 0;
}
//---------------------------------------------------------------------------
