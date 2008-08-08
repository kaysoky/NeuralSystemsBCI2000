/* (C) 2000-2008, BCI2000 Project
/* http://www.bci2000.org
/*/
#include "PCHIncludes.h"
#pragma hdrstop
#include <vcl.h>
//---------------------------------------------------------------------------
USEFORM("UMain.cpp", fMain);
USEFORM("UConnectionInfo.cpp", fConnectionInfo);
USEFORM("UEditMatrix.cpp", fEditMatrix);
USEFORM("UOperatorCfg.cpp", fConfig);
USEFORM("UPreferences.cpp", fPreferences);
USEFORM("UShowParameters.cpp", fShowParameters);
USEFORM("UShowStates.cpp", fShowStates);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
  const char appName[] = "Operator";
  HANDLE appMutex = ::CreateMutex( NULL, TRUE, appName );
  if( ::GetLastError() == ERROR_ALREADY_EXISTS )
  {
    if( appMutex != NULL )
      ::CloseHandle( appMutex );

    Application->Title = "";
       HWND runningApp = ::FindWindow( "TApplication", appName );
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
      Application->Initialize();
      Application->Title = appName;
      Application->CreateForm(__classid(TfMain), &fMain);
                 Application->CreateForm(__classid(TfConnectionInfo), &fConnectionInfo);
                 Application->CreateForm(__classid(TfEditMatrix), &fEditMatrix);
                 Application->CreateForm(__classid(TfConfig), &fConfig);
                 Application->CreateForm(__classid(TfPreferences), &fPreferences);
                 Application->CreateForm(__classid(TfShowParameters), &fShowParameters);
                 Application->CreateForm(__classid(TfShowStates), &fShowStates);
                 Application->Run();
    }
    catch (Exception &exception)
    {
      Application->ShowException(&exception);
    }
    ::ReleaseMutex( appMutex );
    ::CloseHandle( appMutex );
  }
  return 0;
}
//---------------------------------------------------------------------------
