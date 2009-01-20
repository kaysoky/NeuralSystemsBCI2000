#ifdef _NO_VCL

#include "CoreModule.h"
int main( int argc, char** argv )
{
  bool success = CoreModule().Run( argc, argv );
  return ( success ? 0 : -1 );
}

#else // _NO_VCL

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
    Application->Title = "BioSemi Signal Source";
    CoreModuleVCL().Run( _argc, _argv );
  }
  catch (Exception &exception)
  {
    Application->ShowException(&exception);
  }
  return 0;
}

#endif // _NO_VCL
