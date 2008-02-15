////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Environment and EnvironmentExtension are mix-in base classes that
//              channel access to enviroment-like
//              global objects of types ParamList, StateList,
//              StateVector, and provides convenient accessor functions
//              and checking utilities.
//              The difference between Environment and EnvironmentExtension is that
//              Environment descendants are assumed to perform globally relevant
//              actions inside their constructors (as GenericFilter does), while
//              EnvironmentExtension descendants
//              are supposed to use a separate function Publish() for such
//              purposes.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "Environment.h"

#include "StateList.h"
#include "MessageHandler.h"
#include "SysCommand.h"
#include "BCIError.h"
#include "MeasurementUnits.h"
#include "ClassName.h"
#include <sstream>
#include <typeinfo>

using namespace std;
using namespace bci;

////////////////////////////////////////////////////////////////////////////////
// EnvironmentBase definitions
////////////////////////////////////////////////////////////////////////////////
#undef paramlist_
#undef statelist_
#undef statevector_
#undef operator_
#undef phase_
ParamList*   EnvironmentBase::paramlist_ = NULL;
StateList*   EnvironmentBase::statelist_ = NULL;
StateVector* EnvironmentBase::statevector_ = NULL;
ostream*     EnvironmentBase::operator_ = NULL;
EnvironmentBase::ExecutionPhase EnvironmentBase::phase_ = EnvironmentBase::nonaccess;

EnvironmentBase::paramlistAccessor   EnvironmentBase::Parameters;
EnvironmentBase::statelistAccessor   EnvironmentBase::States;
EnvironmentBase::statevectorAccessor EnvironmentBase::Statevector;
EnvironmentBase::operatorAccessor    EnvironmentBase::Operator;

int EnvironmentBase::sNumInstances = 0;
const void* EnvironmentBase::sObjectContext = NULL;

EnvironmentBase::ExtensionsContainer&
EnvironmentBase::Extensions()
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

// Helper function to construct and set an error context string.
void
EnvironmentBase::ErrorContext( const std::string& inContext )
{
  if( inContext.empty() )
    ObjectContext( NULL );
  BCIError::OutStream::SetContext( inContext );
}

// Maintaining an object context to keep track of Parameter/State access.
void
EnvironmentBase::ObjectContext( const void* inContext )
{
  sObjectContext = inContext;
}

const void*
EnvironmentBase::ObjectContext()
{
  return sObjectContext;
}

// Convenient accessor functions.
// Read/write access a parameter by its name.
// New syntax: Separate brackets for indices (nested matrices).
ParamRef
EnvironmentBase::Parameter( const string& name ) const
{
  Param* param = NULL;
  if( Parameters == NULL )
    bcierr_ << "Attempted parameter access during non-access phase."
            << endl;
  else
  {
    if( Parameters->Exists( name ) )
    {
      ParamAccess( name );
      param = &( *Parameters )[ name ];
    }
    else
      bcierr_ << "Parameter \"" << name << "\" does not exist."
              << endl;
  }
  return ParamRef( param );
}

ParamRef
EnvironmentBase::OptionalParameter( const string& name, const string& defaultValue ) const
{
  ParamAccess( name );

  static Param defaultParam;

  Param* param = NULL;
  if( Parameters == NULL )
    bcierr_ << "Attempted parameter access during non-access phase."
            << endl;
  else if( Parameters->Exists( name ) )
    param = &( *Parameters )[ name ];
  else
  {
    defaultParam.Value() = defaultValue;
    param = &defaultParam;
  }
  return ParamRef( param );
}

ParamRef
EnvironmentBase::OptionalParameter( const string& name, double defaultValue ) const
{
  ostringstream oss;
  oss << defaultValue;
  return OptionalParameter( name, oss.str() );
}

string
EnvironmentBase::DescribeValue( const Param& inParam, size_t inIdx1, size_t inIdx2 )
{
  ostringstream oss;
  oss << "Parameter \""
      << inParam.Section() << "->" << inParam.Name()
      << "\": Value";

  if( string( inParam.Type() ).find( "matrix" ) != string::npos )
    oss << " at index (" << inIdx1 << ", " << inIdx2 << ")";
  else if( string( inParam.Type() ).find( "list" ) != string::npos )
    oss << " at index " << inIdx1;

  return oss.str();
}

void
EnvironmentBase::ParamAccess( const string& inName ) const
{
  const void* objectContext = ObjectContext() ? ObjectContext() : this;
  NameSet& accessedParams = ParamsAccessedDuringPreflight()[ objectContext ];
  if( Phase() == preflight )
    accessedParams.insert( inName );
  OnParamAccess( inName );
}

bool
EnvironmentBase::PreflightCondition_( const char* inConditionString,
                                             bool inConditionValue ) const
{
  if( !inConditionValue )
    bcierr_ << "A necessary condition is violated. "
            << "Please make sure that the following is true: "
            << inConditionString << endl;
  return inConditionValue;
}

// Read/write access to a state by its name.
StateRef
EnvironmentBase::State( const std::string& name ) const
{
  int location = 0,
      length = 0;
  const StateList* statelist = NULL;

  if( phase_ != processing )
    statelist = States;
  else if( Statevector != NULL )
    statelist = &Statevector->StateList();

  if( statelist == NULL )
    bcierr_ << "States are inaccessible at this time."
            << endl;
  else
  {
    if( !statelist->Exists( name ) )
      bcierr_ << "State \"" << name << "\" is inaccessible." << endl;
    else
    {
      StateAccess( name );
      const class State& state = ( *statelist )[ name ];
      location = state.Location();
      length = state.Length();
      if( length < 1 )
        bcierr_ << "State \"" << name << "\" has zero length." << endl;
    }
  }
  return StateRef( Statevector, location, length );
}

// Read access to an optional state by its name.
const StateRef
EnvironmentBase::OptionalState( const std::string& name, short defaultValue ) const
{
  StateAccess( name );

  int location = 0,
      length = 0;
  StateVector* statevector = NULL;
  const StateList* statelist = NULL;

  if( phase_ != processing )
    statelist = States;
  else if( Statevector != NULL )
    statelist = &Statevector->StateList();

  if( statelist == NULL )
    bcierr_ << "States are inaccessible at this time."
            << endl;
  else if( statelist->Exists( name ) )
  {
    const class State& state = ( *statelist )[ name ];
    statevector = Statevector;
    location = state.Location();
    length = state.Length();
    if( length < 1 )
      bcierr_ << "State \"" << name << "\" has zero length." << endl;
  }
  return StateRef( statevector, location, length, defaultValue );
}

void
EnvironmentBase::StateAccess( const string& inName ) const
{
  const void* objectContext = ObjectContext() ? ObjectContext() : this;
  NameSet& accessedStates = StatesAccessedDuringPreflight()[ objectContext ];
  if( Phase() == preflight )
    accessedStates.insert( inName );
  OnStateAccess( inName );
}

// Called to prevent access.
void EnvironmentBase::EnterNonaccessPhase()
{
  bcierr__.SetFlushHandler( BCIError::LogicError );
  bciout__.SetFlushHandler( BCIError::Warning );
  switch( phase_ )
  {
    case nonaccess:
      bcierr << "Already in non-access phase" << endl;
      break;
    case construction:
    case preflight:
      break;
    case initialization:
      for( ExtensionsContainer::iterator i = Extensions().begin(); i != Extensions().end(); ++i )
      {
        ErrorContext( "PostInitialize", *i );
        ( *i )->PostInitialize();
      }
      break;
    case startRun:
      for( ExtensionsContainer::iterator i = Extensions().begin(); i != Extensions().end(); ++i )
      {
        ErrorContext( "PostStartRun", *i );
        ( *i )->PostStartRun();
      }
      break;
    case processing:
      for( ExtensionsContainer::iterator i = Extensions().begin(); i != Extensions().end(); ++i )
      {
        ErrorContext( "PostProcess", *i );
        ( *i )->PostProcess();
      }
      break;
    case stopRun:
      for( ExtensionsContainer::iterator i = Extensions().begin(); i != Extensions().end(); ++i )
      {
        ErrorContext( "PostStopRun", *i );
        ( *i )->PostStopRun();
      }
      if( paramlist_ && operator_ )
      {
        ParamList changedParameters;
        for( int i = 0; i < paramlist_->Size(); ++i )
          if( ( *paramlist_ )[ i ].Changed() )
            changedParameters.Add( ( *paramlist_ )[ i ] );
        if( !changedParameters.Empty() )
          if( !(
            MessageHandler::PutMessage( *operator_, changedParameters )
            && MessageHandler::PutMessage( *operator_, SysCommand::EndOfParameter )
          ) )
            bcierr << "Could not publish changed parameters" << endl;
      }
      break;
    case resting:
      break;
    default:
      bcierr << "Unknown execution phase" << endl;
  }
  phase_ = nonaccess;
  paramlist_ = NULL;
  statelist_ = NULL;
  statevector_ = NULL;
  operator_ = NULL;
}
// Called from the framework before any Environment descendant class
// is instantiated.
void EnvironmentBase::EnterConstructionPhase( ParamList*   inParamList,
                                              StateList*   inStateList,
                                              StateVector* inStateVector,
                                              ostream*     inOperator )
{
  bcierr__.SetFlushHandler( BCIError::LogicError );
  bciout__.SetFlushHandler( BCIError::Warning );
  bcidbg__.SetFlushHandler( BCIError::DebugMessage );
  phase_ = construction;
  paramlist_ = inParamList;
  statelist_ = inStateList;
  statevector_ = inStateVector;
  operator_ = inOperator;
  if( paramlist_->Exists( "DebugLevel" ) )
    BCIError::OutStream::SetDebugLevel(
      ::atoi( ( *paramlist_ )[ "DebugLevel" ].Value().c_str() )
    );
  OwnedParams().clear();
  OwnedStates().clear();

  for( ExtensionsContainer::iterator i = Extensions().begin(); i != Extensions().end(); ++i )
  {
    ErrorContext( "Publish", *i );
    ( *i )->Publish();
  }
  ErrorContext( "" );
}

// Called before any call to GenericFilter::Preflight().
void EnvironmentBase::EnterPreflightPhase( ParamList*   inParamList,
                                           StateList*   inStateList,
                                           StateVector* /*inStateVector*/,
                                           ostream*     inOperator )
{
  bcierr__.SetFlushHandler( BCIError::ConfigurationError );
  bciout__.SetFlushHandler( BCIError::Warning );
  phase_ = preflight;
  paramlist_ = inParamList;
  statelist_ = inStateList;
  statevector_ = NULL;
  operator_ = inOperator;
  if( paramlist_->Exists( "DebugLevel" ) )
    BCIError::OutStream::SetDebugLevel(
      ::atoi( ( *paramlist_ )[ "DebugLevel" ].Value().c_str() )
    );

  ParamsRangeChecked().clear();
  if( Parameters != NULL )
  {
    for( NameSetMap::const_iterator i = OwnedParams().begin(); i != OwnedParams().end(); ++i )
      for( NameSet::const_iterator j = i->second.begin(); j != i->second.end(); ++j )
      {
        const Param& p = ( *Parameters )[ *j ];
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
                         << value << ", exceeds lower range (" << lowRange << ")"
                         << endl;
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
                         << value << ", exceeds high range (" << highRange << ")"
                         << endl;
            }
        }
        if( checkLowRange || checkHighRange )
          ParamsRangeChecked().insert( p.Name() );
      }
  }
  ParamsAccessedDuringPreflight().clear();
  StatesAccessedDuringPreflight().clear();

  double SamplingRate = 0,
         SampleBlockSize = 0,
         SourceChGain = 0;

  if( Parameters != NULL )
  {
    if( Parameters->Exists( "SamplingRate" ) )
      SamplingRate = ::atof( ( *Parameters )[ "SamplingRate" ].Value().c_str() );
    if( Parameters->Exists( "SampleBlockSize" ) )
      SampleBlockSize = ::atof( ( *Parameters )[ "SampleBlockSize" ].Value().c_str() );
    if( Parameters->Exists( "SourceChGain" ) )
      SourceChGain = ::atof( ( *Parameters )[ "SourceChGain" ].Value( 0 ).c_str() );
  }

  if( SampleBlockSize > 0 )
    MeasurementUnits::InitializeTimeUnit( SamplingRate / SampleBlockSize );
  if( SamplingRate > 0 )
    MeasurementUnits::InitializeFreqUnit( 1.0 / SamplingRate );
  if( SourceChGain > 0 )
    MeasurementUnits::InitializeVoltageUnit( 1e6 / SourceChGain );

  for( ExtensionsContainer::iterator i = Extensions().begin(); i != Extensions().end(); ++i )
  {
    ErrorContext( "Preflight", *i );
    ( *i )->Preflight();
  }
  ErrorContext( "" );
}

// Called before any call to GenericFilter::Initialize().
void EnvironmentBase::EnterInitializationPhase( ParamList*   inParamList,
                                                StateList*   inStateList,
                                                StateVector* inStateVector,
                                                ostream*     inOperator )
{
  bcierr__.SetFlushHandler( BCIError::RuntimeError );
  bciout__.SetFlushHandler( BCIError::Warning );
  phase_ = initialization;
  paramlist_ = inParamList;
  statelist_ = inStateList;
  statevector_ = inStateVector;
  operator_ = inOperator;
  for( ExtensionsContainer::iterator i = Extensions().begin(); i != Extensions().end(); ++i )
  {
    ErrorContext( "Initialize", *i );
    ( *i )->Initialize();
  }
  ErrorContext( "" );
}

// Called before any call to GenericFilter::StartRun().
void EnvironmentBase::EnterStartRunPhase( ParamList*   inParamList,
                                          StateList*   inStateList,
                                          StateVector* inStateVector,
                                          ostream*     inOperator )
{
  bcierr__.SetFlushHandler( BCIError::RuntimeError );
  bciout__.SetFlushHandler( BCIError::Warning );
  phase_ = startRun;
  paramlist_ = inParamList;
  statelist_ = inStateList;
  statevector_ = inStateVector;
  operator_ = inOperator;
  for( ExtensionsContainer::iterator i = Extensions().begin(); i != Extensions().end(); ++i )
  {
    ErrorContext( "StartRun", *i );
    ( *i )->StartRun();
  }
  ErrorContext( "" );
}

// Called before any call to GenericFilter::Process().
void EnvironmentBase::EnterProcessingPhase( ParamList*   inParamList,
                                            StateList*   inStateList,
                                            StateVector* inStateVector,
                                            ostream*     inOperator )
{
  bcierr__.SetFlushHandler( BCIError::RuntimeError );
  bciout__.SetFlushHandler( BCIError::Warning );
  phase_ = processing;
  paramlist_ = inParamList;
  statelist_ = inStateList;
  statevector_ = inStateVector;
  operator_ = inOperator;
  for( ExtensionsContainer::iterator i = Extensions().begin(); i != Extensions().end(); ++i )
  {
    ErrorContext( "Process", *i );
    ( *i )->Process();
  }
  ErrorContext( "" );
}

// Called before any call to GenericFilter::StopRun().
void EnvironmentBase::EnterStopRunPhase( ParamList*   inParamList,
                                         StateList*   inStateList,
                                         StateVector* inStateVector,
                                         ostream*     inOperator )
{
  bcierr__.SetFlushHandler( BCIError::RuntimeError );
  bciout__.SetFlushHandler( BCIError::Warning );
  phase_ = stopRun;
  paramlist_ = inParamList;
  statelist_ = inStateList;
  statevector_ = inStateVector;
  operator_ = inOperator;
  for( int i = 0; i < paramlist_->Size(); ++i )
    ( *paramlist_ )[ i ].Unchanged();
  for( ExtensionsContainer::iterator i = Extensions().begin(); i != Extensions().end(); ++i )
  {
    ErrorContext( "StopRun", *i );
    ( *i )->StopRun();
  }
  ErrorContext( "" );
}

// Called before any call to GenericFilter::Resting().
void EnvironmentBase::EnterRestingPhase( ParamList*   inParamList,
                                         StateList*   inStateList,
                                         StateVector* inStateVector,
                                         ostream*     inOperator )
{
  bcierr__.SetFlushHandler( BCIError::RuntimeError );
  bciout__.SetFlushHandler( BCIError::Warning );
  phase_ = resting;
  paramlist_ = inParamList;
  statelist_ = inStateList;
  statevector_ = inStateVector;
  operator_ = inOperator;
  for( int i = 0; i < paramlist_->Size(); ++i )
    ( *paramlist_ )[ i ].Unchanged();
  for( ExtensionsContainer::iterator i = Extensions().begin(); i != Extensions().end(); ++i )
  {
    ErrorContext( "Resting", *i );
    ( *i )->Resting();
  }
  ErrorContext( "" );
}

////////////////////////////////////////////////////////////////////////////////
// Environment definitions
////////////////////////////////////////////////////////////////////////////////
void
Environment::OnParamAccess( const string& inName ) const
{
  const void* objectContext = ObjectContext() ? ObjectContext() : this;
  NameSet& accessedParams = ParamsAccessedDuringPreflight()[ objectContext ];
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
  const void* objectContext = ObjectContext() ? ObjectContext() : this;
  NameSet& accessedStates = StatesAccessedDuringPreflight()[ objectContext ],
         & ownedStates = OwnedStates()[ objectContext ];
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


