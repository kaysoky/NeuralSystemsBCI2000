//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
USERES("maxifred.res");
USEFORM("UMain.cpp", fMain);
USEUNIT("channelnames.cpp");
USEFORM("UAbout.cpp", fAbout);
USEFORM("UEditChannellist.cpp", fEditChannellist);
USEFORM("UStateDialog.cpp", OKBottomDlg);
USEFORM("USave2Disk.cpp", fSave2Disk);
USEUNIT("..\SHARED\UBCI2000Data.cpp");
USEUNIT("..\SHARED\UState.cpp");
USEUNIT("..\SHARED\UParameter.cpp");
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
        try
        {
                 Application->Initialize();
                 Application->Title = "Maxifred V3.3";
                 Application->CreateForm(__classid(TfMain), &fMain);
                 Application->CreateForm(__classid(TfAbout), &fAbout);
                 Application->CreateForm(__classid(TfEditChannellist), &fEditChannellist);
                 Application->CreateForm(__classid(TOKBottomDlg), &OKBottomDlg);
                 Application->CreateForm(__classid(TfSave2Disk), &fSave2Disk);
                 Application->Run();
        }
        catch (Exception &exception)
        {
                 Application->ShowException(&exception);
        }
        return 0;
}
//---------------------------------------------------------------------------
