////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: This file contains a template for a registry function.
//   A registry function contains references to global variables representing
//   objects registered via the macros from BCIRegistry.h.
//
//   During compilation, define the REGISTRY_NAME macro to the name of the
//   desired registry function. This must match the name of the cpp file
//   included for the actual object references.
//
// $BEGIN_BCI2000_LICENSE$
//
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2011: BCI2000 team and many external contributors ]
//
// BCI2000 is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// BCI2000 is distributed in the hope that it will be useful, but
//                         WITHOUT ANY WARRANTY
// - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.
//
// $END_BCI2000_LICENSE$
////////////////////////////////////////////////////////////////////////////////
#include "BCIRegistry.h"

#ifndef _MSC_VER
# error This file is needed only when building BCI2000 libraries under MSVC.
#endif // _MSC_VER

#ifndef REGISTRY_NAME
# error Define REGISTRY_NAME to the name of the desired registry function.
#endif // REGISTRY_NAME

#undef STR
#define STR( x ) #x
#undef INCLUDE_FILE_
#define INCLUDE_FILE_( x ) STR( x##.inc )
#undef INCLUDE_FILE
#define INCLUDE_FILE( x ) INCLUDE_FILE_( x )

// Create the definition of a function that references the global variables defined by
// registration macros. Assumes that registration macros have been extracted into a file
// REGISTRY_NAME.cpp.

// We include the named registry file twice:
//  once to create a header section,
//  once to create a function definition.

// Re-define second-level macros to create declarations of global variables.
#undef RegisterFilter_
#define RegisterFilter_( name, pos, priority ) \
   extern "C" void* FilterObjectName_( name, pos, priority );

#undef RegisterExtension_
#define RegisterExtension_( x ) \
   extern "C" void* ExtensionObjectName_( x );

#include INCLUDE_FILE( REGISTRY_NAME )

// Re-define second-level macros to create code referencing global variables.
#undef RegisterFilter_
#define RegisterFilter_( name, pos, priority ) \
   FilterObjectName_( name, pos, priority ) = 0; // will never be executed

#undef RegisterExtension_
#define RegisterExtension_( x ) \
   ExtensionObjectName_( x ) = 0; // will never be executed

// Define a global function called REGISTRY_NAME. Force inclusion of that function
// using MSVC's /include linker switch.
extern "C" void REGISTRY_NAME()
{
#include INCLUDE_FILE( REGISTRY_NAME )
}

