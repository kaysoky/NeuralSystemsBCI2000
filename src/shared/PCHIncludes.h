////////////////////////////////////////////////////////////////////////////////
//
// File: PCHIncludes.h
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// Description: This file contains the header files that need to be included
//              before #pragma hdrstop in place of vcl.h to speed up compiles.
//              Compile with the NO_PCHINCLUDES flag defined to compile without
//              all these headers.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef PCHINCLUDES_H
#define PCHINCLUDES_H

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
