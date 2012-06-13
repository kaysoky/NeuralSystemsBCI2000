////////////////////////////////////////////////////////////////////////////////
// $Id$
// Description: This file declares a purely abstract GenericFilter interface
//   which all BCI2000 filters are supposed to implement.
//
// $BEGIN_BCI2000_LICENSE$
//
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2012: BCI2000 team and many external contributors ]
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
#include "PCHIncludes.h"
#pragma hdrstop

#include "GenericFilter.h"
#include "ClassName.h"
#include <limits>

using namespace std;
using namespace bci;

// GenericFilter class definitions.
size_t GenericFilter::Registrar::sCreatedInstances = 0;
static const string sEmptyPos = "";

GenericFilter::GenericFilter()
{
  AllFilters().push_back( this );
}

GenericFilter::~GenericFilter()
{
  AllFilters().remove( this );
}

#ifdef __BORLANDC__
# pragma warn -8104 // No warning about local statics.
#endif // __BORLANDC__

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

string
GenericFilter::VisParamName() const
{
  return string( "Visualize" ) + ClassName( typeid( *this ) );
}

// GenericFilter::Registrar definitions
GenericFilter::Registrar::Registrar( const char* inPos, int inPriority, bool inAutoDelete )
: mPos( inPos ),
  mPriority( inPriority ),
  mInstance( sCreatedInstances++ )
{
  int maxPriority = inPriority;
  RegistrarSet_::iterator i = Registrars().begin();
  while( i != Registrars().end() )
  {
    RegistrarSet_::iterator j = i++;
    // Determine max priority present.
    if( (*j)->mPriority > maxPriority )
      maxPriority = (*j)->mPriority;
    // Remove all registrars with lower priority.
    if( (*j)->mPriority < inPriority )
      Registrars().erase( j );
  }
  // Only insert if priority is high enough.
  if( inPriority >= maxPriority )
    Registrars().insert( this );

  if( inAutoDelete )
    AutoDeleteInstance().insert( this );
}

GenericFilter::Registrar::~Registrar()
{
  Registrars().erase( this );
}

bool
GenericFilter::Registrar::less::operator()( const Registrar* a, const Registrar* b ) const
{
  if( a->mPos != b->mPos )
    return ( a->mPos < b->mPos );
  return ( a->mInstance < b->mInstance );
}


GenericFilter::RegistrarSet&
GenericFilter::Registrar::Registrars()
{
  static RegistrarSet registrars;
  return registrars;
}

GenericFilter::Registrar::AutoDeleteSet::~AutoDeleteSet()
{
  while( !empty() )
  {
    delete *begin();
    erase( begin() );
  }
}

GenericFilter::Registrar::AutoDeleteSet&
GenericFilter::Registrar::AutoDeleteInstance()
{
  static AutoDeleteSet instance;
  return instance;
}

// GenericFilter definitions
const string&
GenericFilter::GetFirstFilterPosition()
{
  if( Registrar::Registrars().empty() )
    return sEmptyPos;
  return ( *Registrar::Registrars().begin() )->Position();
}

const string&
GenericFilter::GetLastFilterPosition()
{
  if( Registrar::Registrars().empty() )
    return sEmptyPos;
  return ( *Registrar::Registrars().rbegin() )->Position();
}

GenericFilter::ChainInfo
GenericFilter::GetChainInfo()
{
  ChainInfo result;
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
GenericFilter::CallPublish()
{
  ErrorContext( "Publish", this );
  this->Publish();
  ErrorContext( "" );
}

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
    const string& posString = ( *i )->Position();
    ErrorContext( filterName + "::Constructor" );
    GenericFilter* pFilter = ( *i )->NewInstance();
    OwnedFilters().push_front( pFilter );
    if( pFilter->AllowsVisualization() )
    {
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
                               + pFilter->VisParamName()
                               + "= 0 0 0 1 // Visualize "
                               + filterName
                               + " output (boolean)";
      Param param( paramDefinition );
      pFilter->Parameters->Add( param, sortingHint );
      bcidbg( 10 ) << "Added visualization parameter " << param.Name()
                   << " with sorting hint " << sortingHint
                   << "/pos string " << posString
                   << endl;
    }
    ErrorContext( "" );
  }
  for( FiltersType::iterator i = OwnedFilters().begin(); i != OwnedFilters().end(); ++i )
    ( *i )->CallPublish();
}

void
GenericFilter::DisposeFilters()
{
  for( FiltersType::iterator i = OwnedFilters().begin(); i != OwnedFilters().end(); ++i )
  {
    ErrorContext( "Destructor", *i );
    delete *i;
    ErrorContext( "" );
  }
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
    currentFilter->OptionalParameter( currentFilter->VisParamName() );
    currentInput = &OwnedSignals()[ currentFilter ].Properties();
  }
  Output = OwnedSignals()[ currentFilter ].Properties();
}

void
GenericFilter::InitializeFilters()
{
  const GenericSignal::ValueType NaN = numeric_limits<GenericSignal::ValueType>::quiet_NaN();
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
      visEnabled = int( currentFilter->Parameter( currentFilter->VisParamName() ) ) != 0;
      if( visEnabled )
        Visualizations()[ currentFilter ].Send( currentOutput )
                                         .Send( GenericSignal( currentOutput, NaN ) );
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
