#include "PCHIncludes.h"
#pragma hdrstop
#include <vcl.h>
//---------------------------------------------------------------------------
USEFORM("UMain.cpp", fMain);
USEFORM("UAbout.cpp", fAbout);
USEFORM("UConnectionInfo.cpp", fConnectionInfo);
USEFORM("UEditMatrix.cpp", fEditMatrix);
USEFORM("UOperatorCfg.cpp", fConfig);
USEFORM("UPreferences.cpp", fPreferences);
USEFORM("UShowParameters.cpp", fShowParameters);
USEFORM("UShowStates.cpp", fShowStates);
USEFORM("..\shared\UVisConfig.cpp", fVisConfig);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
        try
        {
                 Application->Initialize();
                 Application->Title = "Operator";
                 Application->CreateForm(__classid(TfMain), &fMain);
         Application->CreateForm(__classid(TfAbout), &fAbout);
         Application->CreateForm(__classid(TfConnectionInfo), &fConnectionInfo);
         Application->CreateForm(__classid(TfEditMatrix), &fEditMatrix);
         Application->CreateForm(__classid(TfConfig), &fConfig);
         Application->CreateForm(__classid(TfPreferences), &fPreferences);
         Application->CreateForm(__classid(TfShowParameters), &fShowParameters);
         Application->CreateForm(__classid(TfShowStates), &fShowStates);
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
