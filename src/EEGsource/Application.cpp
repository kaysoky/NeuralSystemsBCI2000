//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
USEFORM("UMain.cpp", fMain);
USEUNIT("GenericADC.cpp");
USEUNIT("RandomNumberADC.cpp");
USEFORM("..\..\shared\V0.17\UVisConfig.cpp", fVisConfig);
USEUNIT("..\..\shared\V0.17\UBCItime.cpp");
USEUNIT("..\..\shared\V0.17\UBitRate.cpp");
USEUNIT("..\..\shared\V0.17\UCoreComm.cpp");
USEUNIT("..\..\shared\V0.17\UCoreMessage.cpp");
USEUNIT("..\..\shared\V0.17\UGenericFilter.cpp");
USEUNIT("..\..\shared\V0.17\UGenericSignal.cpp");
USEUNIT("..\..\shared\V0.17\UGenericVisualization.cpp");
USEUNIT("..\..\shared\V0.17\UParameter.cpp");
USEUNIT("..\..\shared\V0.17\UState.cpp");
USEUNIT("..\..\shared\V0.17\UStatus.cpp");
USEUNIT("..\..\shared\V0.17\USysCommand.cpp");
USEUNIT("..\..\shared\V0.17\UBCI2000Data.cpp");
USEUNIT("UStorage.cpp");
USEUNIT("..\..\shared\V0.17\BCIDirectry.cpp");
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
        try
        {
                 Application->Initialize();
                 Application->Title = "EEGsource V0.20";
                 Application->CreateForm(__classid(TfMain), &fMain);
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
