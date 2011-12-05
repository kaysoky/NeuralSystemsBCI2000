/* (C) 2000-2010, BCI2000 Project
 * http://www.bci2000.org
 */
#include "PCHIncludes.h"
#pragma hdrstop

#ifdef __BORLANDC__

#include <vcl.h>
#include "CoreModuleVCL.h"

#else // __BORLANDC__

// Not using borland, we'll use QT instead
#include "CoreModuleQT.h"

#endif // __BORLANDC__

//---------------------------------------------------------------------------
#ifdef _WIN32

int WINAPI
WinMain( HINSTANCE, HINSTANCE, LPSTR, int )
{
#ifdef __BORLANDC__

        try
        {
                 Application->Initialize();
                 Application->Title = "Neuralynx Source Module";
                 CoreModuleVCL().Run( _argc, _argv );
        }
        catch (Exception &exception)
        {
                 Application->ShowException(&exception);
        }
        return 0;

#else // __BORLANDC__

  bool success = CoreModuleQT().Run( __argc, __argv );
  return success ? 0 : -1;

#endif // __BORLANDC__
}

#else // _WIN32

int main( int argc, char *argv[] )
{
  bool success = CoreModuleQT().Run( argc, argv );
  return success ? 0 : -1;
}

#endif // _WIN32
//---------------------------------------------------------------------------
