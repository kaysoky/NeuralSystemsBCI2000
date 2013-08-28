////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Environment is a mix-in base class
//   that channels access to enviroment-like
//   global objects of types ParamList, StateList,
//   StateVector, and provides convenient accessor functions which perform
//   checks for parameter/state existence, and bounds.
//
//   Inheriting from Environment, EnvironmentExtension is an interface class
//   for components that need to handle system-wide events such as Preflight,
//   Initialize, etc. Direct inheritance from Environment should be used
//   for components that need access to parameters and states without handling
//   global events.
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

#include "Environment.h"

#include "ParamList.h"
#include "StateList.h"
#include "MessageHandler.h"
#include "PhysicalUnit.h"
#include "MeasurementUnits.h"
#include "RunManager.h"
#include "BCIEvent.h"
#include "BCIAssert.h"
#include "BCIStream.h"

#include <cstdlib>

using namespace std;
using namespace bci;

////////////////////////////////////////////////////////////////////////////////
// EnvironmentBase definitions
////////////////////////////////////////////////////////////////////////////////
template<> ParamList* EnvironmentBase::Accessor_<ParamList>::spGlobal = NULL;
template<> StateList* EnvironmentBase::Accessor_<StateList>::spGlobal = NULL;
template<> StateVector* EnvironmentBase::Accessor_<StateVector>::spGlobal = NULL;

#undef AutoConfig_
#undef phase_
EnvironmentBase::ExecutionPhase EnvironmentBase::phase_ = EnvironmentBase::nonaccess;
int EnvironmentBase::sMaxInstanceID = 0;
OSThreadLocal<const EnvironmentBase*> EnvironmentBase::stObjectContext;
OSThreadLocal<const EnvironmentBase*> EnvironmentBase::stWrapperContext;

#ifdef __BORLANDC__
# pragma warn -8104 // No warning about local statics.
#endif // __BORLANDC__

EnvironmentBase::ExtensionsContainer&
EnvironmentBase::Extensions()
{
  static EnvironmentBase::ExtensionsContainer instance;
  return instance;
}

EnvironmentBase::ExtensionsContainer&
EnvironmentBase::ExtensionsPublished()
{
  static EnvironmentBase::ExtensionsContainer instance;
  return instance;
}

EnvironmentBase::NameSetMap&
EnvironmentBase::OwnedParams()
{
  static EnvironmentBase::NameSetMap instance;
  return instance;
}

EnvironmentBase::NameSetMap&
EnvironmentBase::OwnedStates()
{
  static EnvironmentBase::NameSetMap instance;
  return instance;
}

EnvironmentBase::NameSet&
EnvironmentBase::ParamsRangeChecked()
{
  static EnvironmentBase::NameSet instance;
  return instance;
}

EnvironmentBase::NameSetMap&
EnvironmentBase::ParamsAccessedDuringPreflight()
{
  static EnvironmentBase::NameSetMap instance;
  return instance;
}

EnvironmentBase::NameSetMap&
EnvironmentBase::StatesAccessedDuringPreflight()
{
  static EnvironmentBase::NameSetMap instance;
  return instance;
}

// Constructors
EnvironmentBase::EnvironmentBase()
: mInstance( ++sMaxInstanceID ),
  mAutoConfig( false ),
  Parameters( NULL ),
  States( NULL ),
  Statevector( NULL )
{
}

EnvironmentBase::EnvironmentBase( ParamList& rParameters, StateList& rStates, StateVector& rStatevector )
: mInstance( ++sMaxInstanceID ),
  mAutoConfig( false ),
  Parameters( &rParameters ),
  States( &rStates ),
  Statevector( &rStatevector )
{
}

// Destructor
EnvironmentBase::~EnvironmentBase()
{
}

// Helper function to construct and set a context string for displaying errors.
void
EnvironmentBase::ErrorContext( const std::string& inQualifier, const EnvironmentBase* inpObject )
{
  stObjectContext = inpObject;
  string context;
  if( inpObject != NULL )
  {
    context += bci::ClassName( typeid( *inpObject ) );
    context += "::";
  }
  context += inQualifier;
  BCIStream::OutStream::SetContext( context );
}

// Maintaining an object context to keep track of Parameter/State access.
const EnvironmentBase*
EnvironmentBase::ObjectContext() const
{
  return stObjectContext ? static_cast<const EnvironmentBase*>( stObjectContext ) : this;
}

bool
EnvironmentBase::IsGlobalEnvironment() const
{
  bool result =
   Parameters == Accessor_<ParamList>::spGlobal
   && States == Accessor_<StateList>::spGlobal
   && Statevector == Accessor_<StateVector>::spGlobal;
  return result;
}

void
EnvironmentBase::RangeCheckParams( const ParamList* inParamList, const NameSet& inCheck )
{
  for( NameSetMap::const_iterator i = OwnedParams().begin(); i != OwnedParams().end(); ++i )
  {
    vector<string> inters( inCheck.size() );
    vector<string>::iterator inters_end = set_intersection(
      inCheck.begin(), inCheck.end(),
      i->second.begin(), i->second.end(),
      inters.begin(), Param::NameCmp()
    );
    for( vector<string>::const_iterator j = inters.begin(); j != inters_end; ++j )
    {
      const Param& p = ( *inParamList )[ *j ];
      const string& lowRangeStr = p.LowRange(),
                  & highRangeStr = p.HighRange();
      bool checkLowRange = ( !lowRangeStr.empty() ),
           checkHighRange = ( !highRangeStr.empty() );
      if( checkLowRange )
      {
        double lowRange = ::atof( lowRangeStr.c_str() );
        for( int j = 0; j < p.NumRows(); ++j )
          for( int k = 0; k < p.NumColumns(); ++k )
          {
            double value = ::atof( p.Value( j, k ).ToString().c_str() );
            if( value < lowRange )
              bcierr__ << DescribeValue( p, j, k )
                       << " is "
                       << value << ", exceeds lower range (" << lowRange << ")";
          }
      }
      if( checkHighRange )
      {
        double highRange = ::atof( highRangeStr.c_str() );
        for( int j = 0; j < p.NumRows(); ++j )
          for( int k = 0; k < p.NumColumns(); ++k )
          {
            double value = ::atof( p.Value( j, k ).ToString().c_str() );
            if( value > highRange )
              bcierr__ << DescribeValue( p, j, k )
                       << " is "
                       << value << ", exceeds high range (" << highRange << ")";
          }
      }
      if( checkLowRange || checkHighRange )
        ParamsRangeChecked().insert( p.Name() );
    }
  }
}

// Convenient accessor functions.

// Read/write access to a parameter by its name.
// Use an additional pair of brackets for indices.
MutableParamRef
EnvironmentBase::Parameter( const string& inName )
{
  Param* pParam = ParamAccess( inName );
  return MutableParamRef( pParam );
}

ParamRef
EnvironmentBase::Parameter( const string& inName ) const
{
  return ParamRef( ParamAccess( inName ) );
}

ParamRef
EnvironmentBase::ActualParameter( const string& inName ) const
{
  return ParamRef( ParamAccess( inName, actual ) );
}

MutableParamRef
EnvironmentBase::OptionalParameter( const string& inName, const string& inDefaultValue )
{
  Param* pParam = ParamAccess( inName, optional );
  if( !pParam )
  {
    mTemporaryParams[inName].Value() = inDefaultValue;
    pParam = &mTemporaryParams[inName];
  }
  return MutableParamRef( pParam );
}

ParamRef
EnvironmentBase::OptionalParameter( const string& inName, const string& inDefaultValue ) const
{
  return const_cast<EnvironmentBase*>( this )->OptionalParameter( inName, inDefaultValue );
}

MutableParamRef
EnvironmentBase::OptionalParameter( const string& inName, double inDefaultValue )
{
  ostringstream oss;
  oss << inDefaultValue;
  return OptionalParameter( inName, oss.str() );
}

ParamRef
EnvironmentBase::OptionalParameter( const string& inName, double inDefaultValue ) const
{
  return const_cast<EnvironmentBase*>( this )->OptionalParameter( inName, inDefaultValue );
}

string
EnvironmentBase::DescribeValue( const Param& inParam, size_t inIdx1, size_t inIdx2 )
{
  ostringstream oss;
  oss << "Parameter \""
      << inParam.Section() << "->" << inParam.Name()
      << "\": Value";

  if( string( inParam.Type() ).find( "matrix" ) != string::npos )
    oss << " at index (" << inIdx1 + 1 << ", " << inIdx2 + 1 << ")";
  else if( string( inParam.Type() ).find( "list" ) != string::npos )
    oss << " at index " << inIdx1 + 1;

  return oss.str();
}

Param*
EnvironmentBase::ParamAccess( const string& inName, int inFlags ) const
{
  if( IsGlobalEnvironment() )
  {
    NameSet& accessedParams = ParamsAccessedDuringPreflight()[ObjectContext()];
    if( Phase() == preflight )
      accessedParams.insert( inName );
    OnParamAccess( inName );
  }

  Param* pParam = 0;
  if( Parameters == 0 )
    bcierr_ << "Attempted parameter access during non-access phase.";
  else if( Parameters->Exists( inName ) )
    pParam = &( *Parameters )[inName];
  else if( !( inFlags & optional ) )
    bcierr_ << "Parameter \"" << inName << "\" does not exist.";

  if( mAutoConfig && pParam )
  {
    bool inAutoSet = ( mAutoConfigParams.find( inName ) != mAutoConfigParams.end() ),
         mayWrite = inAutoSet;

    const NameSet& ownedParams = OwnedParams()[ObjectContext()];
    if( ownedParams.find( inName ) == ownedParams.end() )
    {
      if( !inAutoSet )
        bcierr_ << "Parameter " << inName << " is inaccessible. "
                << "From AutoConfig(), a filter may only access its own parameters.";
      mAutoConfigParams.insert( inName );
      mayWrite = false;
    }
    if( !mayWrite && !mTemporaryParams.Exists( inName ) )
    {
      mayWrite = IsAutoConfigParam( *pParam );
      if( mayWrite )
        mAutoConfigParams.insert( inName );
      else
        mTemporaryParams[inName] = *pParam;
    }
    if( !mayWrite && !( inFlags & actual ) )
      pParam = &mTemporaryParams[inName];
  }
  return pParam;
}

bool
EnvironmentBase::IsAutoConfigParam( const Param& p )
{
  enum { autoType = 0, autoTag, autoKind, count };
  static const char* s[][count] =
  {
    { "blob", 0 },
    { "auto", "AutoConfig", 0 },
    { "(enumeration)", 0 },
  };
  struct
  {
    bool Is( int type, const char* str )
    { 
      const char** p = s[type];
      while( *p )
        if( !::stricmp( *p++, str ) )
          return true;
      return false;
    }
    bool Contains( int type, const char* str )
    { 
      while( *str )
        if( Is( type, str++ ) )
          return true;
      return false;
    }
  } strings;

  bool result = strings.Is( autoType, p.Type().c_str() );
  if( p.NumValues() == 1 )
    result = result || strings.Is( autoTag, p.Value().c_str() );
  if( p.NumValues() > 0 && !::atoi( p.Value().c_str() ) )
    result = result || strings.Contains( autoKind, p.Comment().c_str() )
                       && strings.Contains( autoTag, p.Comment().c_str() );
  return result;
}

bool
EnvironmentBase::AutoConfig_( bool inAutoConfig )
{
  bool result = mAutoConfig;
  if( mAutoConfig && !inAutoConfig )
    RangeCheckParams( Parameters, mAutoConfigParams );
  mAutoConfigParams.clear();
  mTemporaryParams.Clear();
  mAutoConfig = inAutoConfig;
  return result;
}

bool
EnvironmentBase::PreflightCondition_( const char* inConditionString,
                                             bool inConditionValue ) const
{
  if( !inConditionValue )
    bcierr_ << "A necessary condition is violated. "
            << "Please make sure that the following is true: "
            << inConditionString;
  return inConditionValue;
}


// Read/write access to a state by its name.
StateRef
EnvironmentBase::State( const std::string& inName ) const
{
  const class State* pState = NULL;
  const class StateList* pStatelist = StateListAccess();

  if( pStatelist != NULL )
  {
    if( !pStatelist->Exists( inName ) )
      bcierr_ << "State \"" << inName << "\" is inaccessible.";
    else
    {
      StateAccess( inName );
      pState = &( *pStatelist )[ inName ];
      if( pState->Length() < 1 )
        bcierr_ << "State \"" << inName << "\" has zero length.";
    }
  }
  return StateRef( pState, Statevector, 0 );
}

// Read access to an optional state by its name.
StateRef
EnvironmentBase::OptionalState( const std::string& inName, State::ValueType inDefaultValue ) const
{
  StateAccess( inName );

  const class State* pState = NULL;
  StateVector* pStatevector = NULL;
  const StateList* pStatelist = StateListAccess();

  if( pStatelist != NULL && pStatelist->Exists( inName ) )
  {
    pState = &( *pStatelist )[ inName ];
    pStatevector = Statevector;
    if( pState->Length() < 1 )
      bcierr_ << "State \"" << inName << "\" has zero length.";
  }
  return StateRef( pState, pStatevector, 0, inDefaultValue );
}

const StateList*
EnvironmentBase::StateListAccess() const
{
  const StateList* pStatelist = NULL;
  if( IsGlobalEnvironment() )
  {
    if( phase_ != processing )
      pStatelist = States;
    else if( Statevector != NULL )
      pStatelist = &Statevector->StateList();
  }
  else if( Statevector != NULL )
    pStatelist = &Statevector->StateList();
  else
    pStatelist = States;

  if( pStatelist == NULL )
    bcierr_ << "States are inaccessible at this time.";

  return pStatelist;
}

void
EnvironmentBase::StateAccess( const string& inName ) const
{
  if( IsGlobalEnvironment() )
  {
    NameSet& accessedStates = StatesAccessedDuringPreflight()[ObjectContext()];
    if( Phase() == preflight )
      accessedStates.insert( inName );
    OnStateAccess( inName );
  }
}

// Called to prevent access.
void EnvironmentBase::EnterNonaccessPhase()
{
  switch( phase_ )
  {
    case nonaccess:
      bcierr << "Already in non-access phase";
      break;
    case construction:
      for( ExtensionsContainer::iterator i = Extensions().begin(); i != Extensions().end(); ++i )
      {
        if( ExtensionsPublished().find( *i ) == ExtensionsPublished().end() )
        {
          ( *i )->CallPublish();
          ExtensionsPublished().insert( *i );
        }
      }
      break;
    case preflight:
      break;
    case initialization:
      for( ExtensionsContainer::iterator i = Extensions().begin(); i != Extensions().end(); ++i )
        ( *i )->CallPostInitialize();
      break;
    case startRun:
      for( ExtensionsContainer::iterator i = Extensions().begin(); i != Extensions().end(); ++i )
        ( *i )->CallPostStartRun();
      break;
    case processing:
      for( ExtensionsContainer::iterator i = Extensions().begin(); i != Extensions().end(); ++i )
        ( *i )->CallPostProcess();
      break;
    case stopRun:
      for( ExtensionsContainer::iterator i = Extensions().begin(); i != Extensions().end(); ++i )
        ( *i )->CallPostStopRun();
      BCIEvent::DenyEvents();
      break;
    case resting:
      break;
    default:
      bcierr << "Unknown execution phase";
  }
  bcierr__.SetAction( BCIStream::LogicError );
  bciwarn__.SetAction( BCIStream::Warning );
  bciout__.SetAction( BCIStream::PlainMessage );
  bciout__.SetAction( BCIStream::DebugMessage );
  phase_ = nonaccess;
  Accessor_<ParamList>::spGlobal = NULL;
  Accessor_<StateList>::spGlobal = NULL;
  Accessor_<StateVector>::spGlobal = NULL;
}
// Called from the framework before any Environment descendant class
// is instantiated.
void EnvironmentBase::EnterConstructionPhase( ParamList*   inParamList,
                                              StateList*   inStateList,
                                              StateVector* inStateVector )
{
  bcierr__.SetAction( BCIStream::LogicError );
  phase_ = construction;
  Accessor_<ParamList>::spGlobal = inParamList;
  Accessor_<StateList>::spGlobal = inStateList;
  Accessor_<StateVector>::spGlobal = inStateVector;
  BCIStream::Apply( *inParamList );
  OwnedParams().clear();
  OwnedStates().clear();

  ExtensionsPublished().clear();
  for( ExtensionsContainer::iterator i = Extensions().begin(); i != Extensions().end(); ++i )
  {
    ( *i )->CallPublish();
    ExtensionsPublished().insert( *i );
  }
}

// Called before any call to GenericFilter::Preflight().
void EnvironmentBase::EnterPreflightPhase( ParamList*   inParamList,
                                           StateList*   inStateList,
                                           StateVector* /*inStateVector*/ )
{
  bcierr__.SetAction( BCIStream::ConfigurationError );
  phase_ = preflight;
  Accessor_<ParamList>::spGlobal = inParamList;
  Accessor_<StateList>::spGlobal = inStateList;
  Accessor_<StateVector>::spGlobal = NULL;
  BCIStream::Apply( *inParamList );

  ParamsRangeChecked().clear();
  if( inParamList )
  {
    NameSet notAutoConfig;
    for( int i = 0; i < inParamList->Size(); ++i )
    {
      const Param& p = inParamList->ByIndex( i );
      if( !IsAutoConfigParam( p ) )
        notAutoConfig.insert( p.Name() );
    }
    RangeCheckParams( inParamList, notAutoConfig );
  }
  ParamsAccessedDuringPreflight().clear();
  StatesAccessedDuringPreflight().clear();

  for( ExtensionsContainer::iterator i = Extensions().begin(); i != Extensions().end(); ++i )
    ( *i )->CallPreflight();
}

// Called before any call to GenericFilter::Initialize().
void EnvironmentBase::EnterInitializationPhase( ParamList*   inParamList,
                                                StateList*   inStateList,
                                                StateVector* inStateVector )
{
  bcierr__.SetAction( BCIStream::RuntimeError );
  phase_ = initialization;
  Accessor_<ParamList>::spGlobal = inParamList;
  Accessor_<StateList>::spGlobal = inStateList;
  Accessor_<StateVector>::spGlobal = inStateVector;
  for( ExtensionsContainer::iterator i = Extensions().begin(); i != Extensions().end(); ++i )
    ( *i )->CallInitialize();
}

// Called before any call to GenericFilter::StartRun().
void EnvironmentBase::EnterStartRunPhase( ParamList*   inParamList,
                                          StateList*   inStateList,
                                          StateVector* inStateVector )
{
  bcierr__.SetAction( BCIStream::RuntimeError );
  BCIEvent::AllowEvents();
  phase_ = startRun;
  Accessor_<ParamList>::spGlobal = inParamList;
  Accessor_<StateList>::spGlobal = inStateList;
  Accessor_<StateVector>::spGlobal = inStateVector;
  for( ExtensionsContainer::iterator i = Extensions().begin(); i != Extensions().end(); ++i )
    ( *i )->CallStartRun();
}

// Called before any call to GenericFilter::Process().
void EnvironmentBase::EnterProcessingPhase( ParamList*   inParamList,
                                            StateList*   inStateList,
                                            StateVector* inStateVector )
{
  bcierr__.SetAction( BCIStream::RuntimeError );
  phase_ = processing;
  Accessor_<ParamList>::spGlobal = inParamList;
  Accessor_<StateList>::spGlobal = inStateList;
  Accessor_<StateVector>::spGlobal = inStateVector;
  for( ExtensionsContainer::iterator i = Extensions().begin(); i != Extensions().end(); ++i )
    ( *i )->CallProcess();
}

// Called before any call to GenericFilter::StopRun().
void EnvironmentBase::EnterStopRunPhase( ParamList*   inParamList,
                                         StateList*   inStateList,
                                         StateVector* inStateVector )
{
  bcierr__.SetAction( BCIStream::RuntimeError );
  phase_ = stopRun;
  Accessor_<ParamList>::spGlobal = inParamList;
  Accessor_<StateList>::spGlobal = inStateList;
  Accessor_<StateVector>::spGlobal = inStateVector;
  for( int i = 0; i < inParamList->Size(); ++i )
    ( *inParamList )[ i ].Unchanged();
  for( ExtensionsContainer::iterator i = Extensions().begin(); i != Extensions().end(); ++i )
    ( *i )->CallStopRun();
}

// Called before any call to GenericFilter::Resting().
void EnvironmentBase::EnterRestingPhase( ParamList*   inParamList,
                                         StateList*   inStateList,
                                         StateVector* inStateVector )
{
  bcierr__.SetAction( BCIStream::RuntimeError );
  phase_ = resting;
  Accessor_<ParamList>::spGlobal = inParamList;
  Accessor_<StateList>::spGlobal = inStateList;
  Accessor_<StateVector>::spGlobal = inStateVector;
  for( int i = 0; i < inParamList->Size(); ++i )
    ( *inParamList )[ i ].Unchanged();
  for( ExtensionsContainer::iterator i = Extensions().begin(); i != Extensions().end(); ++i )
    ( *i )->CallResting();
}

void EnvironmentBase::OnExit()
{
  bcierr__.SetAction( 0 );
  bciwarn__.SetAction( 0 );
  bciout__.SetAction( 0 );
  bcidbg__.SetAction( 0 );
}

// Publish() helper functions
void EnvironmentBase::AddParameters( const char** inParams, size_t inCount ) const
{
  ::EncodedString className( bci::ClassName( typeid( *this ) ) );       
  ostringstream oss;                                               
  className.WriteToStream( oss, ":" );                                 
  for( size_t i = 0; i < inCount; ++i )   
  {                                                                      
    Param p;                                                             
    istringstream iss( inParams[i] );                              
    if( !( iss >> p ) )                                                  
      bcierr << "error in parameter definition:\n"                       
             << inParams[ i ];                                               
    else                                                                 
    {                                                                    
      p.Sections().push_back( oss.str() );                              
      if( Parameters->Exists( p.Name() ) )                               
          p.AssignValues( ( *Parameters )[ p.Name() ] );                 
      Parameters->Add( p, -Instance() );                                 
      bcidbg( 10 ) << "Registered parameter " << p.Name() << ", "        
                   << "sorting by (" << -Instance() << ","               
                   << p.Sections() << ")" ;                                         
      OwnedParams()[ObjectContext()].insert( p.Name() );                 
    }                                                                    
  }                                                                      
};

void EnvironmentBase::AddStates( const char** inStates, size_t inCount, int inKind ) const
{
  struct
  {
    const char* operator()( int k )
    {
      switch( k )
      {
        case State::StateKind:
          return "state";
        case State::EventKind:
          return "event";
      }
      return "unknown state kind";
    }
  } KindString;

  for( size_t i = 0; i < inCount; ++i )
  {                                                                   
    class State s;                                                    
    istringstream iss( inStates[i] );                           
    if( !( iss >> s ) )                                               
      bcierr << "error in " << KindString( inKind ) << " definition:\n"                        
             << inStates[i];                                            
    else                                                              
    {                                                                 
      s.SetKind( inKind );                                  
      if( States->Exists( s.Name() ) )                                
      {                                                               
        int k = ( *States )[s.Name()].Kind();
        if( k != inKind )      
          bcierr << "trying to define " << KindString( inKind ) << " "                         
                 << s.Name()                                          
                 << ", has been previously defined as " << KindString( k );           
        else                                                          
          ( *States )[s.Name()].AssignValue( s );                   
      }                                                               
      else                                                            
        States->Add( s );                                             
      OwnedStates()[ObjectContext()].insert( s.Name() );              
    }                                                                 
  }                                                                   
};


////////////////////////////////////////////////////////////////////////////////
// Environment definitions
////////////////////////////////////////////////////////////////////////////////
RunManager Environment::sRunManager;

string
Environment::CurrentSession() const
{
  return sRunManager.CurrentSession();
}

string
Environment::CurrentRun() const
{
  return sRunManager.CurrentRun();
}

void
Environment::OnParamAccess( const string& inName ) const
{
  NameSet& accessedParams = ParamsAccessedDuringPreflight()[ObjectContext()];
  switch( EnvironmentBase::Phase() )
  {
    case construction:
    case preflight:
      break;

    default:
      if( accessedParams.find( inName ) == accessedParams.end()
           && ParamsRangeChecked().find( inName ) == ParamsRangeChecked().end() )
        bcierr_ << "Parameter \"" << inName << "\" was accessed during"
                << " initialization or processing, but not checked for"
                << " consistency during preflight phase."
                << endl;
  }
}

void
Environment::OnStateAccess( const string& inName ) const
{
  NameSet& accessedStates = StatesAccessedDuringPreflight()[ObjectContext()],
         & ownedStates = OwnedStates()[ObjectContext()];
  switch( EnvironmentBase::Phase() )
  {
    case construction:
    case preflight:
      break;

    default:
      if( accessedStates.find( inName ) == accessedStates.end()
           && ownedStates.find( inName ) == ownedStates.end() )
        bcierr_ << "State \"" << inName << "\" was accessed during"
                << " initialization or processing, but not checked for"
                << " existence during preflight phase."
                << endl;
  }
}

////////////////////////////////////////////////////////////////////////////////
// EnvironmentExtension definitions
////////////////////////////////////////////////////////////////////////////////
EnvironmentExtension::AutoDeleteSet&
EnvironmentExtension::AutoDeleteInstance()
{
  static AutoDeleteSet instance;
  return instance;
}

EnvironmentExtension*
EnvironmentExtension::AutoDelete( EnvironmentExtension* p )
{
  AutoDeleteInstance().insert( p );
  return p;
}

#define CALL( x, y )                   \
void EnvironmentExtension::Call##x() y \
{                                      \
  ErrorContext( #x, this );            \
  try { this->x(); }                   \
  catch( const BCIException& e )       \
  { bcierr_ << e.What(); }             \
  ErrorContext( "" );                  \
}

CALL( Publish, )
CALL( Preflight, const )
CALL( Initialize, )
CALL( PostInitialize, )
CALL( StartRun, );
CALL( PostStartRun, )
CALL( Process, );
CALL( PostProcess, )
CALL( StopRun, );
CALL( PostStopRun, )
CALL( Resting, );

