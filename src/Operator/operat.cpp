//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
USERES("operat.res");
USEFORM("UMain.cpp", fMain);
USEUNIT("USysStatus.cpp");
USEFORM("UAbout.cpp", fAbout);
USEFORM("UConnectionInfo.cpp", fConnectionInfo);
USEFORM("UEditMatrix.cpp", fEditMatrix);
USEFORM("UOperatorCfg.cpp", fConfig);
USEFORM("UPreferences.cpp", fPreferences);
USEUNIT("UScript.cpp");
USEFORM("UShowParameters.cpp", fShowParameters);
USEFORM("UShowStates.cpp", fShowStates);
USEUNIT("USysLog.cpp");
USEFORM("..\shared\UVisConfig.cpp", fVisConfig);
USEUNIT("..\shared\UBCI2000Data.cpp");
USEUNIT("..\shared\UBCItime.cpp");
USEUNIT("..\shared\UBitRate.cpp");
USEUNIT("..\shared\UCoreComm.cpp");
USEUNIT("..\shared\UCoreMessage.cpp");
USEUNIT("..\shared\UGenericFilter.cpp");
USEUNIT("..\shared\UGenericSignal.cpp");
USEUNIT("..\shared\UGenericVisualization.cpp");
USEUNIT("..\shared\UParameter.cpp");
USEUNIT("..\shared\UState.cpp");
USEUNIT("..\shared\UStatus.cpp");
USEUNIT("..\shared\USysCommand.cpp");
USEUNIT("..\shared\BCIDirectry.cpp");
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
