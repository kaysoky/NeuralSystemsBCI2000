//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
USEFORM("..\UMain.cpp", fMain);
USEFORM("..\..\shared\UVisConfig.cpp", fVisConfig);
USEUNIT("..\..\shared\UBCI2000Data.cpp");
USEUNIT("..\..\shared\UBCItime.cpp");
USEUNIT("..\..\shared\UBitRate.cpp");
USEUNIT("..\..\shared\UCoreComm.cpp");
USEUNIT("..\..\shared\UCoreMessage.cpp");
USEUNIT("..\..\shared\UGenericFilter.cpp");
USEUNIT("..\..\shared\UGenericSignal.cpp");
USEUNIT("..\..\shared\UGenericVisualization.cpp");
USEUNIT("..\..\shared\UParameter.cpp");
USEUNIT("..\..\shared\UState.cpp");
USEUNIT("..\..\shared\UStatus.cpp");
USEUNIT("..\..\shared\USysCommand.cpp");
USEUNIT("..\..\shared\BCIDirectry.cpp");
USEUNIT("Task.cpp");
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
        try
        {
                 Application->Initialize();
                 Application->Title = "Neural Music V0.01";
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
