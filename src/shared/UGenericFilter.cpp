////////////////////////////////////////////////////////////////////////////////
// $Id$
//
// File: UGenericFilter.h
//
// Changes: Oct 21, 2002, juergen.mellinger@uni-tuebingen.de
//          - Made GenericFilter a true base class, and a purely abstract one.
//          Mar 19, 2003, juergen.mellinger@uni-tuebingen.de
//          - Removed references to essentially global objects
//            of types CORECOMM, PARAMLIST, STATELIST, STATEVECTOR, and
//            introduced the Environment class to handle access to those objects.
//          - Added the Preflight() member as a purely virtual function to enforce
//            implementation in subclasses.
//          Jun 10, 2004, juergen.mellinger@uni-tuebingen.de
//          - Disabled auto-instantiation for signal processing modules in favor
//            of a list of Filter() statements in an additional cpp file
//            to avoid unwanted changes in filter sequence when using the new
//            unified module framework code.
// $Log$
// Revision 1.13  2005/12/20 11:42:41  mellinger
// Added CVS id and log to comment.
//
//
// Description: This file declares a purely abstract GenericFilter interface
//   which all BCI2000 filters are supposed to implement.
//
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "UGenericFilter.h"

using namespace std;

// GenericFilter class definitions.
size_t GenericFilter::Registrar::createdInstances = 0;
string emptyPos = "";

GenericFilter::filters_type&
GenericFilter::AllFilters()
{
  static filters_type allFilters;
  return allFilters;
}

GenericFilter::filters_type&
GenericFilter::OwnedFilters()
{
  static filters_type ownedFilters;
  return ownedFilters;
}

GenericFilter::signals_type&
GenericFilter::OwnedSignals()
{
  static signals_type ownedSignals;
  return ownedSignals;
}

GenericFilter::registrarSet&
GenericFilter::Registrar::Registrars()
{
  static registrarSet registrars;
  return registrars;
}

const string&
GenericFilter::GetFirstFilterPosition()
{
  if( Registrar::Registrars().empty() )
    return emptyPos;
  return ( *Registrar::Registrars().begin() )->GetPosition();
}

const string&
GenericFilter::GetLastFilterPosition()
{
  if( Registrar::Registrars().empty() )
    return emptyPos;
  return ( *Registrar::Registrars().rbegin() )->GetPosition();
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
  for( registrarSet::reverse_iterator i = Registrar::Registrars().rbegin();
                                       i != Registrar::Registrars().rend(); ++i )
    OwnedFilters().push_front( ( *i )->NewInstance() );
}

void
GenericFilter::DisposeFilters()
{
  for( filters_type::iterator i = OwnedFilters().begin(); i != OwnedFilters().end(); ++i )
    delete *i;
  OwnedFilters().clear();
}

void
GenericFilter::PreflightFilters( const SignalProperties& Input,
                                       SignalProperties& Output )
{
  OwnedSignals()[ NULL ].SetProperties( Input );
  GenericFilter* currentFilter = NULL;
  const SignalProperties* currentInput = &Input;
  for( filters_type::iterator i = OwnedFilters().begin(); i != OwnedFilters().end(); ++i )
  {
    currentFilter = *i;
    SignalProperties currentOutput;
    currentFilter->Preflight( *currentInput, currentOutput );
    // The output signal will be created here if it does not exist.
    OwnedSignals()[ currentFilter ].SetProperties( currentOutput );
    currentInput = &OwnedSignals()[ currentFilter ].GetProperties();
  }
  Output = OwnedSignals()[ currentFilter ].GetProperties();
}

void
GenericFilter::InitializeFilters()
{
  const SignalProperties* currentInput = &OwnedSignals()[ NULL ].GetProperties();
  GenericFilter* currentFilter = NULL;
  for( filters_type::iterator i = OwnedFilters().begin(); i != OwnedFilters().end(); ++i )
  {
    currentFilter = *i;
    // This will implicitly create the output signal if it does not exist.
    currentFilter->Initialize2( *currentInput, OwnedSignals()[ currentFilter ].GetProperties() );
    currentInput = &OwnedSignals()[ currentFilter ].GetProperties();
  }
}

void
GenericFilter::StartRunFilters()
{
  for( filters_type::iterator i = OwnedFilters().begin(); i != OwnedFilters().end(); ++i )
    ( *i )->StartRun();
}

void
GenericFilter::ProcessFilters( const GenericSignal* Input,
                                     GenericSignal* Output )
{
  const GenericSignal* currentInput = Input;
  GenericFilter* currentFilter = NULL;
  for( filters_type::iterator i = OwnedFilters().begin(); i != OwnedFilters().end(); ++i )
  {
    currentFilter = *i;
    // This will implicitly create the output signal if it does not exist.
    currentFilter->Process( currentInput, &OwnedSignals()[ currentFilter ] );
    currentInput = &OwnedSignals()[ currentFilter ];
  }
  if( currentFilter )
    *Output = OwnedSignals()[ currentFilter ];
  else
    *Output = *Input;
}

void
GenericFilter::StopRunFilters()
{
  for( filters_type::iterator i = OwnedFilters().begin(); i != OwnedFilters().end(); ++i )
    ( *i )->StopRun();
}

void
GenericFilter::RestingFilters()
{
  for( filters_type::iterator i = OwnedFilters().begin(); i != OwnedFilters().end(); ++i )
    ( *i )->Resting();
}

void
GenericFilter::HaltFilters()
{
  for( filters_type::iterator i = OwnedFilters().begin(); i != OwnedFilters().end(); ++i )
    ( *i )->Halt();
}

// Create an instance of the same type as a given one.
GenericFilter*
GenericFilter::NewInstance( const GenericFilter* existingInstance )
{
  Registrar* registrarFound = NULL;
  registrarSet::iterator i = Registrar::Registrars().begin();
  while( i != Registrar::Registrars().end() && registrarFound == NULL )
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

