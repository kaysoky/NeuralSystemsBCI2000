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
GenericFilter::filters_type GenericFilter::allFilters;
GenericFilter::filters_type GenericFilter::ownedFilters;
GenericFilter::signals_type GenericFilter::ownedSignals;
GenericFilter::registrarSet GenericFilter::Registrar::registrars;
size_t GenericFilter::Registrar::createdInstances = 0;
string emptyPos = "";

const string&
GenericFilter::GetFirstFilterPosition()
{
  if( Registrar::registrars.empty() )
    return emptyPos;
  return ( *Registrar::registrars.begin() )->GetPosition();
}

const string&
GenericFilter::GetLastFilterPosition()
{
  if( Registrar::registrars.empty() )
    return emptyPos;
  return ( *Registrar::registrars.rbegin() )->GetPosition();
}

// Instantiate all registered filters once.
// We iterate through the registrars in reverse order. This implies that
// sub-filters acquired via PassFilter<>() must have a position string greater
// than that of their master filter; this appears more intuitive because
// sub-filters may then share the first part of their position
// strings with the master filters.
void
GenericFilter::InstantiateFilters()
{
  for( registrarSet::reverse_iterator i = Registrar::registrars.rbegin();
                                       i != Registrar::registrars.rend(); ++i )
    ownedFilters.push_front( ( *i )->NewInstance() );
}

void
GenericFilter::DisposeFilters()
{
  for( filters_type::iterator i = ownedFilters.begin(); i != ownedFilters.end(); ++i )
    delete *i;
  ownedFilters.clear();
}

void
GenericFilter::InitializeFilters()
{
  for( filters_type::iterator i = ownedFilters.begin(); i != ownedFilters.end(); ++i )
    ( *i )->Initialize();
}

void
GenericFilter::PreflightFilters( const SignalProperties& Input,
                                       SignalProperties& Output )
{
  const SignalProperties* currentInput = &Input;
  GenericFilter* currentFilter = NULL;
  for( filters_type::iterator i = ownedFilters.begin(); i != ownedFilters.end(); ++i )
  {
    currentFilter = *i;
    // This will implicitly create the output signal if it does not exist.
    currentFilter->Preflight( *currentInput, ownedSignals[ currentFilter ] );
    currentInput = &ownedSignals[ currentFilter ];
  }
  if( currentFilter )
    Output = ownedSignals[ currentFilter ];
  else
    Output = Input;
}

void
GenericFilter::ProcessFilters( const GenericSignal* Input,
                                     GenericSignal* Output )
{
  const GenericSignal* currentInput = Input;
  GenericFilter* currentFilter = NULL;
  for( filters_type::iterator i = ownedFilters.begin(); i != ownedFilters.end(); ++i )
  {
    currentFilter = *i;
    // This will implicitly create the output signal if it does not exist.
    currentFilter->Process( currentInput, &ownedSignals[ currentFilter ] );
    currentInput = &ownedSignals[ currentFilter ];
  }
  if( currentFilter )
    *Output = ownedSignals[ currentFilter ];
  else
    *Output = *Input;
}

void
GenericFilter::RestingFilters()
{
  for( filters_type::iterator i = ownedFilters.begin(); i != ownedFilters.end(); ++i )
    ( *i )->Resting();
}

void
GenericFilter::HaltFilters()
{
  for( filters_type::iterator i = ownedFilters.begin(); i != ownedFilters.end(); ++i )
    ( *i )->Halt();
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

