//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
USERES("operat.res");
USEFORM("UMain.cpp", fMain);
USEFORM("UAbout.cpp", fAbout);
USEFORM("UConnectionInfo.cpp", fConnectionInfo);
USEFORM("UEditMatrix.cpp", fEditMatrix);
USEFORM("UOperatorCfg.cpp", fConfig);
USEFORM("UPreferences.cpp", fPreferences);
USEUNIT("UScript.cpp");
USEFORM("UShowParameters.cpp", fShowParameters);
USEFORM("UShowStates.cpp", fShowStates);
USEUNIT("USysLog.cpp");
USEUNIT("USysStatus.cpp");
USEFORM("..\..\shared\V0.17\UVisConfig.cpp", fVisConfig);
USEUNIT("..\..\shared\V0.17\UBCItime.cpp");
USEUNIT("..\..\shared\V0.17\UBitRate.cpp");
USEUNIT("..\..\shared\V0.17\UCoreComm.cpp");
USEUNIT("..\..\shared\V0.17\UCoreMessage.cpp");
USEUNIT("..\..\shared\V0.17\UGenericFilter.cpp");
USEUNIT("..\..\shared\V0.17\UGenericSignal.cpp");
USEUNIT("..\..\shared\V0.17\UParameter.cpp");
USEUNIT("..\..\shared\V0.17\UState.cpp");
USEUNIT("..\..\shared\V0.17\UStatus.cpp");
USEUNIT("..\..\shared\V0.17\USysCommand.cpp");
USEUNIT("..\..\shared\V0.17\UBCI2000Data.cpp");
USEUNIT("..\..\shared\V0.17\UGenericVisualization.cpp");
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
