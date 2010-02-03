////////////////////////////////////////////////////////////////////////////////
// $Id$
// Description: This file declares a purely abstract GenericFilter interface
//   which all BCI2000 filters are supposed to implement.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "GenericFilter.h"
#include "BCIError.h"
#include "ClassName.h"

using namespace std;
using namespace bci;

// GenericFilter class definitions.
size_t GenericFilter::Registrar::createdInstances = 0;
const string emptyString = "";
const string& emptyPos = emptyString;

GenericFilter::GenericFilter()
{
  if( Environment::Phase() != construction )
    bcierr_ << "GenericFilter descendant of type \""
            << ClassName( typeid( *this ) )
            << "\" instantiated outside construction phase."
            << endl;
  AllFilters().push_back( this );
}

// Static members
GenericFilter::FiltersType&
GenericFilter::AllFilters()
{
  static FiltersType allFilters;
  return allFilters;
}

GenericFilter::FiltersType&
GenericFilter::OwnedFilters()
{
  static FiltersType ownedFilters;
  return ownedFilters;
}

GenericFilter::SignalsType&
GenericFilter::OwnedSignals()
{
  static SignalsType ownedSignals;
  return ownedSignals;
}

GenericFilter::VisualizationsType&
GenericFilter::Visualizations()
{
  static VisualizationsType visualizations;
  return visualizations;
}

const string&
GenericFilter::VisParamName( const GenericFilter* inpFilter )
{
  static string result;
  result = string( "Visualize" ) + ClassName( typeid( *inpFilter ) );
  return result;
}

GenericFilter::RegistrarSet&
GenericFilter::Registrar::Registrars()
{
  static RegistrarSet registrars;
  return registrars;
}

const string&
GenericFilter::GetFirstFilterPosition()
{
  if( Registrar::Registrars().empty() )
    return emptyPos;
  return ( *Registrar::Registrars().begin() )->Position();
}

const string&
GenericFilter::GetLastFilterPosition()
{
  if( Registrar::Registrars().empty() )
    return emptyPos;
  return ( *Registrar::Registrars().rbegin() )->Position();
}

const GenericFilter::ChainInfo&
GenericFilter::GetChainInfo()
{
  static ChainInfo result;
  result.clear();
  for( RegistrarSet::const_iterator i = Registrar::Registrars().begin();
       i != Registrar::Registrars().end(); ++i )
  {
    ChainEntry entry;
    entry.position = ( *i )->Position();
    entry.name = ClassName( ( *i )->Typeid() );
    result.push_back( entry );
  }
  return result;
}

ostream&
GenericFilter::ChainInfo::WriteToStream( ostream& os )
{
  for( const_iterator i = begin(); i != end(); ++i )
    os << "{ " << i->position << " " << i->name << " } ";
  return os;
}

// Wrapper functions for "handler" calls.
void
GenericFilter::CallPreflight( const SignalProperties& Input,
                                    SignalProperties& Output ) const
{
  ErrorContext( "Preflight", this );
  this->Preflight( Input, Output );
  ErrorContext( "" );
}

void
GenericFilter::CallInitialize( const SignalProperties& Input,
                               const SignalProperties& Output )
{
  ErrorContext( "Initialize", this );
  this->Initialize( Input, Output );
  ErrorContext( "" );
}

void
GenericFilter::CallStartRun()
{
  ErrorContext( "StartRun", this );
  this->StartRun();
  ErrorContext( "" );
}

void
GenericFilter::CallStopRun()
{
  ErrorContext( "StopRun", this );
  this->StopRun();
  ErrorContext( "" );
}

void
GenericFilter::CallProcess( const GenericSignal& Input,
                                  GenericSignal& Output )
{
  ErrorContext( "Process", this );
  this->Process( Input, Output );
  ErrorContext( "" );
}

void
GenericFilter::CallResting()
{
  ErrorContext( "Resting", this );
  this->Resting();
  ErrorContext( "" );
}

void
GenericFilter::CallHalt()
{
  ErrorContext( "Halt", this );
  this->Halt();
  ErrorContext( "" );
}


// Instantiate all registered filters once.
// We iterate through the registrars in reverse order. This implies that
// sub-filters acquired via PassFilter<>() must have a position string greater
// than that of their master filter; this appears more intuitive because
// sub-filters may then share the first part of their position
// strings with their master filters.
void
GenericFilter::InstantiateFilters()
{
  for( RegistrarSet::reverse_iterator i = Registrar::Registrars().rbegin();
                                       i != Registrar::Registrars().rend(); ++i )
  {
    string filterName = ClassName( ( *i )->Typeid() );
    ErrorContext( filterName + "::Constructor" );
    GenericFilter* pFilter = ( *i )->NewInstance();
    OwnedFilters().push_front( pFilter );
    if( pFilter->AllowsVisualization() )
    {
      const string& posString = ( *i )->Position();
      Visualizations()[ pFilter ].SetSourceID( posString );
      // Convert the position string to a float value with identical sort order.
      float placeValue = 1,
            sortingHint = 0;
      for( string::const_iterator i = posString.begin(); i != posString.end(); ++i )
      {
        sortingHint += *i * placeValue;
        placeValue /= 1 << ( 8 * sizeof( *i ) );
      }
      string paramDefinition = string( "Visualize:Processing%20Stages" )
                               + " int "
                               + VisParamName( pFilter )
                               + "= 0 0 0 1 // Visualize "
                               + filterName
                               + " output (boolean)";
      pFilter->Parameters->Add( Param( paramDefinition ), sortingHint );
    }
  }
  ErrorContext( "" );
}

void
GenericFilter::DisposeFilters()
{
  for( FiltersType::iterator i = OwnedFilters().begin(); i != OwnedFilters().end(); ++i )
  {
    ErrorContext( "Destructor", *i );
    delete *i;
  }
  ErrorContext( "" );
  OwnedFilters().clear();
  Visualizations().clear();
}

void
GenericFilter::PreflightFilters( const SignalProperties& Input,
                                       SignalProperties& Output )
{
  OwnedSignals()[ NULL ].SetProperties( Input );
  GenericFilter* currentFilter = NULL;
  const SignalProperties* currentInput = &Input;
  for( FiltersType::iterator i = OwnedFilters().begin(); i != OwnedFilters().end(); ++i )
  {
    currentFilter = *i;
    SignalProperties currentOutput;
    currentFilter->CallPreflight( *currentInput, currentOutput );
    if( currentOutput.Name() == currentInput->Name()
        || currentOutput.Name().empty() )
      currentOutput.SetName( ClassName( typeid( *currentFilter ) ) );
    // The output signal will be created here if it does not exist.
    OwnedSignals()[ currentFilter ].SetProperties( currentOutput );
    currentFilter->OptionalParameter( VisParamName( currentFilter ) );
    currentInput = &OwnedSignals()[ currentFilter ].Properties();
  }
  Output = OwnedSignals()[ currentFilter ].Properties();
}

void
GenericFilter::InitializeFilters()
{
  const SignalProperties* currentInput = &OwnedSignals()[ NULL ].Properties();
  GenericFilter* currentFilter = NULL;
  for( FiltersType::iterator i = OwnedFilters().begin(); i != OwnedFilters().end(); ++i )
  {
    currentFilter = *i;
    const SignalProperties& currentOutput = OwnedSignals()[ currentFilter ].Properties();
    // This will implicitly create the output signal if it does not exist.
    currentFilter->CallInitialize( *currentInput, currentOutput );
    // Output signal visualization.
    bool visEnabled = false;
    if( currentFilter->AllowsVisualization() )
    {
      visEnabled = int( currentFilter->Parameter( VisParamName( currentFilter ) ) ) != 0;
      Visualizations()[ currentFilter ].SetEnabled( visEnabled );
      if( visEnabled )
        Visualizations()[ currentFilter ].Send( currentOutput );
      Visualizations()[ currentFilter ].Send( CfgID::Visible, visEnabled );
    }
    Visualizations()[ currentFilter ].SetEnabled( visEnabled );

    currentInput = &OwnedSignals()[ currentFilter ].Properties();
  }
}

void
GenericFilter::StartRunFilters()
{
  for( FiltersType::iterator i = OwnedFilters().begin(); i != OwnedFilters().end(); ++i )
    ( *i )->CallStartRun();
}

void
GenericFilter::ProcessFilters( const GenericSignal& Input,
                                     GenericSignal& Output )
{
  const GenericSignal* currentInput = &Input;
  GenericFilter* currentFilter = NULL;
  for( FiltersType::iterator i = OwnedFilters().begin(); i != OwnedFilters().end(); ++i )
  {
    currentFilter = *i;
    // This will implicitly create the output signal if it does not exist.
    GenericSignal& currentOutput = OwnedSignals()[ currentFilter ];
    currentFilter->CallProcess( *currentInput, currentOutput );
    if( Visualizations()[ currentFilter ].Enabled() )
      Visualizations()[ currentFilter ].Send( currentOutput );

    currentInput = &OwnedSignals()[ currentFilter ];
  }
  if( currentFilter )
    Output = OwnedSignals()[ currentFilter ];
  else
    Output = Input;
}

void
GenericFilter::StopRunFilters()
{
  for( FiltersType::iterator i = OwnedFilters().begin(); i != OwnedFilters().end(); ++i )
    ( *i )->CallStopRun();
}

void
GenericFilter::RestingFilters()
{
  for( FiltersType::iterator i = OwnedFilters().begin(); i != OwnedFilters().end(); ++i )
    ( *i )->CallResting();
}

void
GenericFilter::HaltFilters()
{
  for( FiltersType::iterator i = OwnedFilters().begin(); i != OwnedFilters().end(); ++i )
    ( *i )->CallHalt();
}

// Create an instance of the same type as a given one.
GenericFilter*
GenericFilter::NewInstance( const GenericFilter* existingInstance )
{
  Registrar* registrarFound = NULL;
  RegistrarSet::iterator i = Registrar::Registrars().begin();
  while( i != Registrar::Registrars().end() && registrarFound == NULL )
  {
    if( typeid( *existingInstance ) == ( *i )->Typeid() )
      registrarFound = *i;
    ++i;
  }
  GenericFilter* newInstance = NULL;
  if( registrarFound )
    newInstance = registrarFound->NewInstance();
  return newInstance;
}

