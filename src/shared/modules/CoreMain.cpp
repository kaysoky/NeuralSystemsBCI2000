////////////////////////////////////////////////////////////////////////////////
// $Id$
// Description: main() function definition for core modules running under 
//   non-Win32 systems.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "CoreModule.h"

int main( int argc, char** argv )
{
  bool success = CoreModule().Run( argc, argv );
  return ( success ? 0 : -1 );
}

