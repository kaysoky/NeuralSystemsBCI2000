/* (C) 2000-2010, BCI2000 Project
/* http://www.bci2000.org
/*/
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

//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
        try
        {
                 Application->Initialize();
                 Application->Title = "SignalGenerator Source Module";
                 CoreModuleVCL().Run( _argc, _argv );
        }
        catch (Exception &exception)
        {
                 Application->ShowException(&exception);
        }
        return 0;
}
//---------------------------------------------------------------------------

#endif // _NO_VCL

