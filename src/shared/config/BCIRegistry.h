////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: This header file centralizes definitions of macros that allow
//   for instantiation of Filters and Extensions in their own source files, or
//   in a PipeDefinition.h file.
//
//   Basically, these macros define global instances of FilterRegistrars, or
//   EnvironmentExtensions. When these get linked into the final executable,
//   registrar or extension instances are then available for processing.
//
//   With MSVC, a problem arises in conjunction with static libraries because
//   MSVC strips all unused symbols from a static library when linking to it.
//   Thus, for MSVC static libraries we need a tool that extracts registration
//   macros from source files, and create a registry function which is then
//   forced into the final executable using the MSVC linker's /include switch.
//   Compile BCIRegistry.cpp with REGISTRY_NAME set to the name of the
//   registration function.
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
#ifndef BCI_REGISTRY_H
#define BCI_REGISTRY_H

// Filter() and RegisterFilter() both register filters to the framework.
// However, Filter() statements take precedence over RegisterFilter() statements:
// When any Filter() statement is present in a module, all RegisterFilter() statements are ignored.
// Typically, you have a RegisterFilter() statement in the filter's own cpp file, determining the
// Filter's default position in the filter chain.
// In Signal Processing Modules, there is an additional "PipeDefinition.cpp" containing a global
// filter chain definition in form of Filter() statements. These take precedence over local
// RegisterFilter() statements, so "PipeDefinition.cpp" defines the actual filter chain.
// Also, in Signal Processing Modules, RegisterFilter() has no effect.
#define Filter( name, pos )          RegisterFilter_( name, pos, 2 )
#define RegisterFilter( name, pos )  RegisterFilter_( name, pos, 1 )

// An Extension() macro to register extensions for automatic instantiation.
// Write Extension( MyExtensionClass ); in your extension's cpp file to create
// an instance of it -- unless your extension class is meant to be instantiated
// by other classes, of course.
#define Extension( name )            RegisterExtension_( name )
#define RegisterExtension( name )    RegisterExtension_( name )

// A macro that tells the extract_registry tool which macro calls to extract from source files.
// This macro must list all macros defined above.
#define REGISTRY_MACROS { "Filter", "RegisterFilter", "Extension", "RegisterExtension" }

// Macros to create names of global objects.
#define FilterObjectName_( name, pos, priority ) name##Registrar##priority
#define ExtensionObjectName_( name )             name##Instance

// Define second-level macros to create definitions of global variables.
#if( __GNUC__ ) // gcc has a "used" attribute that retains unreferenced symbols when linking.

# define RegisterFilter_( name, pos, priority )  \
  GenericFilter::FilterRegistrar<name> FilterObjectName_( name, pos, priority ) __attribute__(( used )) ( #pos, priority );

# define RegisterExtension_( x )  \
  x ExtensionObjectName( x ) __attribute__(( used ));

#elif( _MSC_VER ) // MSVC provides linker comments but these are not transitive when linking against static libraries.
                  // We use void* type and "extern C" so we need not deal with name mangling, and can reference objects
                  // from a Registry function without needing access to filters' header files.

# define RegisterFilter_( name, pos, priority )  \
   extern "C" void* FilterObjectName_( name, pos, priority ) = new GenericFilter::FilterRegistrar<name>( #pos, priority, true ); \
   __pragma( comment( linker, "/include:_" #name "Registrar" #priority ) )

# define RegisterExtension_( x )  \
   extern "C" void* ExtensionObjectName_( x ) = EnvironmentExtension::AutoDelete( new x ); \
   __pragma( comment( linker, "/include:_" #x "Instance" ) )

#else // __GNUC__, _MSC_VER // for other compilers, we hope they don't strip unreferenced globals

# define RegisterFilter_( name, pos, priority )  \
  GenericFilter::FilterRegistrar<name> FilterObjectName_( name, pos, priority )( #pos, priority );

# define RegisterExtension_( x )  \
  x ExtensionObjectName_( x );

#endif // __GNUC__, _MSC_VER

#endif // BCI_REGISTRY_H
