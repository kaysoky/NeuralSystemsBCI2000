//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
USEUNIT("..\Statistics.cpp");
USEUNIT("..\classfilter.cpp");
USEUNIT("..\Normalfilter.cpp");
USEUNIT("..\SpatialFilter.cpp");
USEUNIT("..\StatFilter.cpp");
USEUNIT("..\CalibrationFilter.cpp");
USEFORM("..\UMain.cpp", fMain);
USEUNIT("UFilterHandling.cpp");
USEUNIT("GETMEM.cpp");
USEUNIT("TemporalFilter.cpp");
USEUNIT("ARFilter.cpp");
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
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
        try
        {
                 Application->Initialize();
                 Application->Title = "SignalProcessing";
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
