////////////////////////////////////////////////////////////////////////////////
//
// File: UGenericFilter.cpp
//
// Description: Definitions for the GenericFilter interface
//   which all BCI2000 filters are supposed to implement.
//
// Changes: Oct 21, 2002, juergen.mellinger@uni-tuebingen.de
//          - Made GenericFilter a true base class, and a purely abstract one.
//          Mar 19, 2003, juergen.mellinger@uni-tuebingen.de
//          - Removed references to essentially global objects
//            of types CORECOMM, PARAMLIST, STATELIST, STATEVECTOR, and
//            introduced the Environment class to handle access to those objects.
//          - Added the Preflight() member as a purely virtual function to enforce
//            implementation in subclasses.
//
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "UGenericFilter.h"

// The #pragma makes the linker evaluate dependencies for startup initalization.
#pragma package(smart_init)

using namespace std;

// GenericFilter class definitions.
GenericFilter::filterSet GenericFilter::filters;
GenericFilter::registrarSet GenericFilter::Registrar::registrars;
size_t GenericFilter::Registrar::createdInstances = 0;

// Instantiate all registered filters once.
void
GenericFilter::InstantiateFilters()
{
  for( registrarSet::iterator i = Registrar::registrars.begin();
                                       i != Registrar::registrars.end(); ++i )
    ( *i )->NewInstance();
}

// Create an instance of the same type as a given one.
GenericFilter*
GenericFilter::NewInstance( const GenericFilter* existingInstance )
{
  Registrar* registrarFound = NULL;
  registrarSet::iterator i = Registrar::registrars.begin();
  while( i != Registrar::registrars.end() && registrarFound == NULL )
  {
    if( typeid( *existingInstance ) == ( *i )->GetTypeid() )
      registrarFound = *i;
    ++i;
  }
  GenericFilter* newInstance = NULL;
  if( registrarFound )
    newInstance = registrarFound->NewInstance();
  return newInstance;
}

