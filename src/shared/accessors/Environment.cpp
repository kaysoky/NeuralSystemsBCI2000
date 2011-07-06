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
#include "PCHIncludes.h"
#pragma hdrstop

#include "Environment.h"

#include "StateList.h"
#include "MessageHandler.h"
#include "SysCommand.h"
#include "BCIError.h"
#include "MeasurementUnits.h"
#include "ClassName.h"
#include "PhysicalUnit.h"

#if IS_APP_MODULE
# include "ApplicationWindow.h"
#endif // IS_APP_MODULE

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
const EnvironmentBase* EnvironmentBase::sObjectContext = NULL;

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

#if IS_APP_MODULE
EnvironmentBase::NameSetMap&
EnvironmentBase::WindowsAccessedDuringPreflight()
{
  static EnvironmentBase::NameSetMap instance;
  return instance;
}
#endif // IS_APP_MODULE

// Helper function to construct and set a context string for displaying errors.
void
EnvironmentBase::ErrorContext( const std::string& inQualifier, const EnvironmentBase* inpObject )
{
  sObjectContext = inpObject;
  string context;
  if( !inQualifier.empty() )
  {
    context = bci::ClassName( typeid( *inpObject ) );
    context += "::";
    context += inQualifier;
  }
  BCIError::OutStream::SetContext( context );
}

// Maintaining an object context to keep track of Parameter/State access.
const EnvironmentBase*
EnvironmentBase::ObjectContext() const
{
  return sObjectContext ? sObjectContext : this;
}

// Convenient accessor functions.

// Read/write access to a parameter by its name.
// Use an additional pair of brackets for indices.
ParamRef
EnvironmentBase::Parameter( const string& inName ) const
{
  Param* pParam = NULL;
  if( Parameters == NULL )
    bcierr_ << "Attempted parameter access during non-access phase."
            << endl;
  else
  {
    if( Parameters->Exists( inName ) )
    {
      ParamAccess( inName );
      pParam = &( *Parameters )[ inName ];
    }
    else
      bcierr_ << "Parameter \"" << inName << "\" does not exist."
              << endl;
  }
  return ParamRef( pParam );
}

ParamRef
EnvironmentBase::OptionalParameter( const string& inName, const string& inDefaultValue ) const
{
  ParamAccess( inName );

  static Param defaultParam;

  Param* pParam = NULL;
  if( Parameters == NULL )
    bcierr_ << "Attempted parameter access during non-access phase."
            << endl;
  else if( Parameters->Exists( inName ) )
    pParam = &( *Parameters )[ inName ];
  else
  {
    defaultParam.Value() = inDefaultValue;
    pParam = &defaultParam;
  }
  return ParamRef( pParam );
}

ParamRef
EnvironmentBase::OptionalParameter( const string& inName, double inDefaultValue ) const
{
  ostringstream oss;
  oss << inDefaultValue;
  return OptionalParameter( inName, oss.str() );
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

void
EnvironmentBase::ParamAccess( const string& inName ) const
{
  NameSet& accessedParams = ParamsAccessedDuringPreflight()[ObjectContext()];
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
EnvironmentBase::State( const std::string& inName ) const
{
  const class State* pState = NULL;
  const class StateList* pStatelist = StateListAccess();

  if( pStatelist != NULL )
  {
    if( !pStatelist->Exists( inName ) )
      bcierr_ << "State \"" << inName << "\" is inaccessible."
              << endl;
    else
    {
      StateAccess( inName );
      pState = &( *pStatelist )[ inName ];
      if( pState->Length() < 1 )
        bcierr_ << "State \"" << inName << "\" has zero length."
                << endl;
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
      bcierr_ << "State \"" << inName << "\" has zero length."
              << endl;
  }
  return StateRef( pState, pStatevector, 0, inDefaultValue );
}

const StateList*
EnvironmentBase::StateListAccess() const
{
  const StateList* pStatelist = NULL;
  if( phase_ != processing )
    pStatelist = States;
  else if( Statevector != NULL )
    pStatelist = &Statevector->StateList();

  if( pStatelist == NULL )
    bcierr_ << "States are inaccessible at this time."
            << endl;

  return pStatelist;
}

void
EnvironmentBase::StateAccess( const string& inName ) const
{
  NameSet& accessedStates = StatesAccessedDuringPreflight()[ObjectContext()];
  if( Phase() == preflight )
    accessedStates.insert( inName );
  OnStateAccess( inName );
}

#if IS_APP_MODULE
void
EnvironmentBase::Window( const string& inName ) const
{ // This function is called during the preflight phase.
  string name = inName;
  if( name.empty() )
    name = ApplicationWindow::DefaultName;

  NameSet& accessedWindows = WindowsAccessedDuringPreflight()[ObjectContext()];
  if( Phase() == preflight )
    accessedWindows.insert( name );

  if( !Windows()->Exists( name ) )
    bcierr_ << "Trying to access non-existing application window \""
            << name
            << "\""
            << endl;
}

ApplicationWindow&
EnvironmentBase::Window( const string& inName )
{ // This function is called outside the preflight phase.
  string name = inName;
  if( name.empty() )
    name = ApplicationWindow::DefaultName;

  NameSet& accessedWindows = WindowsAccessedDuringPreflight()[ObjectContext()];
  if( accessedWindows.find( name ) == accessedWindows.end() )
    bcierr_ << "Application window \"" << name << "\" was accessed during"
            << " initialization or processing, but not checked for"
            << " existence during preflight phase."
            << endl;

  ApplicationWindow* pWindow = ( *Windows() )[name];
  if( pWindow == NULL )
  {
    bcierr_ << "Access to non-existing application window \""
            << name
            << "\""
            << endl;
    pWindow = new ApplicationWindow( name );
  }

  return *pWindow;
}

const ApplicationWindowList*
EnvironmentBase::Windows() const
{
  return &ApplicationWindow::Windows();
}
#endif IS_APP_MODULE


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

  if( Parameters != NULL )
    MeasurementUnits::Initialize( *Parameters );

  for( ExtensionsContainer::iterator i = Extensions().begin(); i != Extensions().end(); ++i )
    ( *i )->CallPreflight();
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
    ( *i )->CallInitialize();
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
    ( *i )->CallStartRun();
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
    ( *i )->CallProcess();
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
    ( *i )->CallStopRun();
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
    ( *i )->CallResting();
}

////////////////////////////////////////////////////////////////////////////////
// Environment definitions
////////////////////////////////////////////////////////////////////////////////
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
#define CALL( x )                    \
void EnvironmentExtension::Call##x() \
{                                    \
  ErrorContext( #x, this );          \
  this->##x();                       \
  ErrorContext( "" );                \
}                                    \

#define CONST_CALL( x )                    \
void EnvironmentExtension::Call##x() const \
{                                          \
  ErrorContext( #x, this );                \
  this->##x();                             \
  ErrorContext( "" );                      \
}                                          \

CALL( Publish )
CONST_CALL( Preflight )
CALL( Initialize )
CALL( PostInitialize )
CALL( StartRun );
CALL( PostStartRun )
CALL( Process );
CALL( PostProcess )
CALL( StopRun );
CALL( PostStopRun )
CALL( Resting );
