// $Id$
#ifndef __BORLANDC__

#include "CoreModule.h"
int main( int argc, char** argv )
{
  bool success = CoreModule().Run( argc, argv );
  return ( success ? 0 : -1 );
}

#if _WIN32
int WINAPI WinMain( HINSTANCE, HINSTANCE, LPSTR, int )
{
  return main( __argc, __argv );
}
#endif // _WIN32

#else // !__BORLANDC__

#include "PCHIncludes.h"
#pragma hdrstop

#include <vcl.h>
#include "CoreModuleVCL.h"

//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
        try
        {
                 Application->Initialize();
                 Application->Title = "";
         \
                 CoreModuleVCL().Run( _argc, _argv );
        }
        catch (Exception &exception)
        {
                 Application->ShowException(&exception);
        }
        return 0;
}
//---------------------------------------------------------------------------

#endif // !__BORLANDC__

