////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: This file contains the header files that need to be included
//              before #pragma hdrstop in place of vcl.h to speed up compiles.
//              Compile with the NO_PCHINCLUDES flag defined to compile without
//              all these headers.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef PCHINCLUDES_H
#define PCHINCLUDES_H

// Suppress "never used value" warning on variable initialization
#pragma warn -8004

#ifndef NO_PCHINCLUDES

#include <vcl.h>

// STL headers
#include <map>
#include <set>
#include <list>
#include <vector>
#include <string>

#endif // NO_PCHINCLUDES

#endif // PCHINCLUDES_H
