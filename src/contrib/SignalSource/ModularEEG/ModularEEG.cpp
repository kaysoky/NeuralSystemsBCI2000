#include "PCHIncludes.h"
#pragma hdrstop

#include <vcl.h>
#include "CoreModuleVCL.h"

WINAPI
WinMain( HINSTANCE, HINSTANCE, LPSTR, int )
{
  try
  {
    Application->Initialize();
    Application->Title = "ModularEEG Signal Source";
    CoreModuleVCL().Run( _argc, _argv );
  }
  catch (Exception &exception)
  {
    Application->ShowException(&exception);
  }
  return 0;
}


