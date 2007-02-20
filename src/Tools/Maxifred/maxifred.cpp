/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
USEFORM("UMain.cpp", fMain);
USEFORM("UAbout.cpp", fAbout);
USEFORM("UEditChannellist.cpp", fEditChannellist);
USEFORM("UStateDialog.cpp", OKBottomDlg);
USEFORM("USave2Disk.cpp", fSave2Disk);
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


