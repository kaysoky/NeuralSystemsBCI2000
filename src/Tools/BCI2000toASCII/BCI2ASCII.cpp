//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
USERES("BCI2ASCII.res");
USEFORM("UMain.cpp", fMain);
USELIB("C:\Program Files\MATLAB_SR11\extern\include\libmx.lib");
USELIB("C:\Program Files\MATLAB_SR11\extern\include\libmat.lib");
USELIB("C:\Program Files\MATLAB_SR11\extern\include\libeng.lib");
USEUNIT("..\..\SHARED\UState.cpp");
USEUNIT("..\..\SHARED\UParameter.cpp");
USEUNIT("..\..\SHARED\UBCI2000Data.cpp");
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
        try
        {
                 Application->Initialize();
                 Application->Title = "BCI2000toASCII";
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
