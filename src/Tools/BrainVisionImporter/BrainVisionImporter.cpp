//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
//---------------------------------------------------------------------------
USEFORM("MainForm.cpp", ImporterForm);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    HANDLE appMutex = ::CreateMutex( NULL, TRUE, APP_NAME );
    if( ::GetLastError() == ERROR_ALREADY_EXISTS )
    {
      if( appMutex != NULL )
        ::CloseHandle( appMutex );

      Application->Title = "";
      HWND runningApp = ::FindWindow( "TApplication", APP_NAME );
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
        Application->Title = APP_NAME;
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
