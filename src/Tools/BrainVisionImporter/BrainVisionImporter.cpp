/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
//---------------------------------------------------------------------------
USEFORM("MainForm.cpp", ImporterForm);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    AnsiString appName = ::ChangeFileExt( ::ExtractFileName( __FILE__ ), "" );
    HANDLE appMutex = ::CreateMutex( NULL, TRUE, appName.c_str() );
    if( ::GetLastError() == ERROR_ALREADY_EXISTS )
    {
      if( appMutex != NULL )
        ::CloseHandle( appMutex );

      Application->Title = "";
         HWND runningApp = ::FindWindow( "TApplication", appName.c_str() );
      if( runningApp != NULL )
      {
        ::SendMessage( runningApp, WM_SYSCOMMAND, SC_RESTORE, 0 );
        ::SetForegroundWindow( runningApp );
      }
    }
    else
    {
      try
      {
        Application->Title = appName;
        Application->Initialize();
        Application->CreateForm(__classid(TImporterForm), &ImporterForm);
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
      ::ReleaseMutex( appMutex );
      ::CloseHandle( appMutex );
    }
    return 0;
}
//---------------------------------------------------------------------------


