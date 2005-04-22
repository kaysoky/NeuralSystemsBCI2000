////////////////////////////////////////////////////////////////////////////////
//
// File: UEnvironment.cpp
//
// Description: EnvironmentBase and Environment are mix-in base classes that
//              channel access to enviroment-like
//              global objects of types PARAMLIST, STATELIST,
//              STATEVECTOR, and provides convenient accessor functions
//              and checking utilities.
//              The difference between EnvironmentBase and Environment is that
//              Environment descendants are assumed to perform globally relevant
//              actions inside their constructors (as GenericFilter does), while
//              EnvironmentBase descendants such as Environment::Extension
//              are supposed to use a separate function Publish() for such
//              purposes. 
//
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "UEnvironment.h"

#include "MessageHandler.h"
#include "USysCommand.h"
#include "UBCIError.h"
#include <typeinfo>

using namespace std;

////////////////////////////////////////////////////////////////////////////////
// EnvironmentBase definitions
////////////////////////////////////////////////////////////////////////////////
#undef _paramlist
#undef _statelist
#undef _statevector
#undef _operator
#undef _phase
PARAMLIST*   EnvironmentBase::_paramlist = NULL;
STATELIST*   EnvironmentBase::_statelist = NULL;
STATEVECTOR* EnvironmentBase::_statevector = NULL;
ostream*     EnvironmentBase::_operator = NULL;
EnvironmentBase::executionPhase EnvironmentBase::_phase = EnvironmentBase::nonaccess;

EnvironmentBase::paramlistAccessor   EnvironmentBase::Parameters;
EnvironmentBase::statelistAccessor   EnvironmentBase::States;
EnvironmentBase::statevectorAccessor EnvironmentBase::Statevector;
EnvironmentBase::operatorAccessor    EnvironmentBase::Operator;

EnvironmentBase::ExtensionsContainer&
EnvironmentBase::Extensions()
{
  static EnvironmentBase::ExtensionsContainer instance;
  return instance;
}

PARAM*
EnvironmentBase::GetParamPtr( const string& name ) const
{
#ifdef TODO
# error Check for Preflight access before Initialize/Process access
#endif // TODO
  PARAM* param = NULL;

  if( Parameters == NULL )
    _bcierr << "Tried parameter access during non-access phase."
            << endl;
  else
  {
    param = Parameters->GetParamPtr( name.c_str() );
    if( param == NULL )
    {
      _bcierr << "Parameter \"" << name << "\" is inaccessible."
              << endl;
    }
  }
  return param;
}

PARAM*
EnvironmentBase::GetOptionalParamPtr( const string& name ) const
{
  PARAM* param = NULL;

  if( Parameters == NULL )
    _bcierr << "Tried parameter access during non-access phase."
            << endl;
  else
    param = Parameters->GetParamPtr( name.c_str() );
  return param;
}

void
EnvironmentBase::CheckRange( const PARAM* param,
                             size_t row, size_t column ) const
{
#ifdef TODO
# error Ensure reasonable ranges before enabling the range check.
#endif // TODO
#if 0
  if( row > param->GetNumValuesDimension1() || column > param->GetNumValuesDimension2() )
    _bcierr << "Parameter \"" << param->GetName() << "\": "
            << "Index out of bounds (" << row << ", " << column << ")."
            << endl;
  double lowRange = atof( param->GetLowRange() ),
         highRange = atof( param->GetHighRange() ),
         value = atof( param->GetValue( row, column ) );
  // Do not check the range if the range is not numeric.
  if( lowRange < highRange &&
      ( value < lowRange || value > highRange ) )
      _bcierr << "Parameter \"" << param->GetName() << "\" is out of range."
              << endl;
#endif
}

// Convenient accessor functions.
// Read/write access a parameter by its name and indices, if applicable.
PARAM::type_adapter
EnvironmentBase::Parameter( const string& name,
                            size_t row, size_t column ) const
{
#ifdef TODO
# error Range Check for all values of a parameter if row == column == 0
#endif // TODO
  PARAM* param = GetParamPtr( name );
  if( param != NULL )
      CheckRange( param, row, column );
  return PARAM::type_adapter( param, row, column );
}

PARAM::type_adapter
EnvironmentBase::Parameter( const string& name,
                            const string& rowLabel, size_t column ) const
{
  PARAM* param = GetParamPtr( name );
  size_t row = 0;
  if( param != NULL )
  {
      row = param->LabelsDimension1()[ rowLabel ];
      CheckRange( param, row, column );
  }
  return PARAM::type_adapter( param, row, column );
}

PARAM::type_adapter
EnvironmentBase::Parameter( const string& name,
                            size_t row, const string& columnLabel ) const
{
  PARAM* param = GetParamPtr( name );
  size_t column = 0;
  if( param != NULL )
  {
      column = param->LabelsDimension2()[ columnLabel ];
      CheckRange( param, row, column );
  }
  return PARAM::type_adapter( param, row, column );
}

PARAM::type_adapter
EnvironmentBase::Parameter( const string& name,
                            const string& rowLabel, const string& columnLabel ) const
{
  PARAM* param = GetParamPtr( name );
  size_t row = 0,
         column = 0;
  if( param != NULL )
  {
      row = param->LabelsDimension1()[ rowLabel ];
      column = param->LabelsDimension2()[ columnLabel ];
      CheckRange( param, row, column );
  }
  return PARAM::type_adapter( param, row, column );
}

const PARAM::type_adapter
EnvironmentBase::OptionalParameter( double defaultValue,
                                    PARAM* param,
                                    size_t row, size_t column ) const
{
  ostringstream os;
  os << defaultValue;
  return OptionalParameter( os.str(), param, row, column );
}

const PARAM::type_adapter
EnvironmentBase::OptionalParameter( const string& defaultValue,
                                    PARAM* inParam,
                                    size_t row, size_t column ) const
{
  static PARAM defaultParam;
  PARAM* param = &defaultParam;
  if( inParam == NULL )
    defaultParam.SetValue( defaultValue, row, column );
  else
  {
    param = inParam;
    CheckRange( param, row, column );
  }
  return PARAM::type_adapter( param, row, column );
}

const PARAM::type_adapter
EnvironmentBase::OptionalParameter( double defaultValue,
                                    const string& name,
                                    size_t row,
                                    size_t column ) const
{
  return OptionalParameter( defaultValue, GetOptionalParamPtr( name ), row, column );
}

const PARAM::type_adapter
EnvironmentBase::OptionalParameter( const string& name,
                                    size_t row,
                                    size_t column ) const
{
  return OptionalParameter( "", GetOptionalParamPtr( name ), row, column );
}

const PARAM::type_adapter
EnvironmentBase::OptionalParameter( const string& name,
                                    const string& rowLabel, size_t column ) const
{
  PARAM* param = GetOptionalParamPtr( name );
  size_t row = 0;
  if( param != NULL )
      row = param->LabelsDimension1()[ rowLabel ];
  return OptionalParameter( "", param, row, column );
}

const PARAM::type_adapter
EnvironmentBase::OptionalParameter( const string& name,
                                    size_t row, const string& columnLabel ) const
{
  PARAM* param = GetOptionalParamPtr( name );
  size_t column = 0;
  if( param != NULL )
      column = param->LabelsDimension2()[ columnLabel ];
  return OptionalParameter( "", param, row, column );
}

const PARAM::type_adapter
EnvironmentBase::OptionalParameter( const string& name,
                                    const string& rowLabel, const string& columnLabel ) const
{
  PARAM* param = GetOptionalParamPtr( name );
  size_t row = 0,
         column = 0;
  if( param != NULL )
  {
      row = param->LabelsDimension1()[ rowLabel ];
      column = param->LabelsDimension2()[ columnLabel ];
  }
  return OptionalParameter( "", param, row, column );
}

const PARAM::type_adapter
EnvironmentBase::OptionalParameter( double defaultValue,
                                    const string& name,
                                    const string& rowLabel, size_t column ) const
{
  PARAM* param = GetOptionalParamPtr( name );
  size_t row = 0;
  if( param != NULL )
      row = param->LabelsDimension1()[ rowLabel ];
  return OptionalParameter( defaultValue, param, row, column );
}

const PARAM::type_adapter
EnvironmentBase::OptionalParameter( double defaultValue,
                                    const string& name,
                                    size_t row, const string& columnLabel ) const
{
  PARAM* param = GetOptionalParamPtr( name );
  size_t column = 0;
  if( param != NULL )
      column = param->LabelsDimension2()[ columnLabel ];
  return OptionalParameter( defaultValue, param, row, column );
}

const PARAM::type_adapter
EnvironmentBase::OptionalParameter( double defaultValue,
                                    const string& name,
                                    const string& rowLabel, const string& columnLabel ) const
{
  PARAM* param = GetOptionalParamPtr( name );
  size_t row = 0,
         column = 0;
  if( param != NULL )
  {
      row = param->LabelsDimension1()[ rowLabel ];
      column = param->LabelsDimension2()[ columnLabel ];
  }
  return OptionalParameter( defaultValue, param, row, column );
}

bool
EnvironmentBase::_PreflightCondition( const char* inConditionString,
                                      bool inConditionValue ) const
{
  if( !inConditionValue )
    _bcierr << "A necessary condition is violated. "
            << "Please make sure that the following is true: "
            << inConditionString << endl;
  return inConditionValue;
}


// Read/write access to a state by its name.
STATEVECTOR::type_adapter
EnvironmentBase::State( const char* name ) const
{
#ifdef TODO
# error In the type adapter, check whether the value fits into the state.
#endif // TODO
  int bitLocation = 0,
      byteLocation = 0,
      length = 0;
  const STATELIST* statelist = NULL;

  if( _phase != processing )
    statelist = States;
  else if( Statevector != NULL )
    statelist = Statevector->GetStateListPtr();

  if( statelist == NULL )
    _bcierr << "States are inaccessible at this time."
            << endl;
  else
  {
    STATE* state = statelist->GetStatePtr( name );
    if( state == NULL )
      _bcierr << "State \"" << name << "\" is inaccessible." << endl;
    else
    {
      bitLocation = state->GetBitLocation();
      byteLocation = state->GetByteLocation();
      length = state->GetLength();
      if( length < 1 )
        _bcierr << "State \"" << name << "\" has zero length." << endl;
    }
  }
  return STATEVECTOR::type_adapter( Statevector, byteLocation, bitLocation, length );
}

// Read access to an optional state by its name.
const STATEVECTOR::type_adapter
EnvironmentBase::OptionalState( short defaultValue, const char* name ) const
{
  int bitLocation = 0,
      byteLocation = 0,
      length = 0;
  STATEVECTOR* statevector = NULL;
  const STATELIST* statelist = NULL;

  if( _phase != processing )
    statelist = States;
  else if( Statevector != NULL )
    statelist = Statevector->GetStateListPtr();

  if( statelist == NULL )
    _bcierr << "States are inaccessible at this time."
            << endl;
  else
  {
    STATE* state = statelist->GetStatePtr( name );
    if( state != NULL )
    {
      statevector = Statevector;
      bitLocation = state->GetBitLocation();
      byteLocation = state->GetByteLocation();
      length = state->GetLength();
      if( length < 1 )
        _bcierr << "State \"" << name << "\" has zero length." << endl;
    }
  }
  return STATEVECTOR::type_adapter( statevector, byteLocation, bitLocation, length, defaultValue );
}

// Called to prevent access.
void EnvironmentBase::EnterNonaccessPhase()
{
  __bcierr.SetFlushHandler( BCIError::LogicError );
  __bciout.SetFlushHandler( BCIError::Warning );
  switch( _phase )
  {
    case nonaccess:
      bcierr << "Already in non-access phase" << endl;
      break;
    case construction:
    case preflight:
    case initialization:
    case processing:
      break;
    case resting:
      if( _paramlist && _operator )
      {
        PARAMLIST changedParameters;
        for( PARAMLIST::iterator i = _paramlist->begin(); i != _paramlist->end(); ++i )
          if( i->second.Changed() )
            changedParameters.insert( *i );
        if( !changedParameters.empty() )
          if( !(
            MessageHandler::PutMessage( *_operator, changedParameters )
            && MessageHandler::PutMessage( *_operator, SYSCMD::EndOfParameter )
          ) )
            bcierr << "Could not publish changed parameters" << endl;
      }
      break;
    default:
      bcierr << "Unknown execution phase" << endl;
  }
  _phase = nonaccess;
  _paramlist = NULL;
  _statelist = NULL;
  _statevector = NULL;
  _operator = NULL;
}
// Called from the framework before any EnvironmentBase descendant class
// is instantiated.
void EnvironmentBase::EnterConstructionPhase( PARAMLIST*   inParamList,
                                              STATELIST*   inStateList,
                                              STATEVECTOR* inStateVector,
                                              ostream*     inOperator )
{
  __bcierr.SetFlushHandler( BCIError::LogicError );
  __bciout.SetFlushHandler( BCIError::Warning );
  _phase = construction;
  _paramlist = inParamList;
  _statelist = inStateList;
  _statevector = inStateVector;
  _operator = inOperator;
  for( ExtensionsContainer::iterator i = Extensions().begin(); i != Extensions().end(); ++i )
    ( *i )->Publish();
}

// Called before any call to GenericFilter::Preflight().
void EnvironmentBase::EnterPreflightPhase( PARAMLIST*   inParamList,
                                           STATELIST*   inStateList,
                                           STATEVECTOR* inStateVector,
                                           ostream*     inOperator )
{
  __bcierr.SetFlushHandler( BCIError::ConfigurationError );
  __bciout.SetFlushHandler( BCIError::Warning );
  _phase = preflight;
  _paramlist = inParamList;
  _statelist = inStateList;
  _statevector = NULL;
  _operator = inOperator;
  for( ExtensionsContainer::iterator i = Extensions().begin(); i != Extensions().end(); ++i )
    ( *i )->Preflight();
}

// Called before any call to GenericFilter::Initialize().
void EnvironmentBase::EnterInitializationPhase( PARAMLIST*   inParamList,
                                                STATELIST*   inStateList,
                                                STATEVECTOR* inStateVector,
                                                ostream*     inOperator )
{
  __bcierr.SetFlushHandler( BCIError::RuntimeError );
  __bciout.SetFlushHandler( BCIError::Warning );
  _phase = initialization;
  _paramlist = inParamList;
  _statelist = inStateList;
  _statevector = inStateVector;
  _operator = inOperator;
  for( ExtensionsContainer::iterator i = Extensions().begin(); i != Extensions().end(); ++i )
    ( *i )->Initialize();
}

// Called before any call to GenericFilter::StartRun().
void EnvironmentBase::EnterStartRunPhase( PARAMLIST*   inParamList,
                                          STATELIST*   inStateList,
                                          STATEVECTOR* inStateVector,
                                          ostream*     inOperator )
{
  __bcierr.SetFlushHandler( BCIError::RuntimeError );
  __bciout.SetFlushHandler( BCIError::Warning );
  _phase = initialization;
  _paramlist = inParamList;
  _statelist = inStateList;
  _statevector = inStateVector;
  _operator = inOperator;
  for( ExtensionsContainer::iterator i = Extensions().begin(); i != Extensions().end(); ++i )
    ( *i )->StartRun();
}

// Called before any call to GenericFilter::Process().
void EnvironmentBase::EnterProcessingPhase( PARAMLIST*   inParamList,
                                            STATELIST*   inStateList,
                                            STATEVECTOR* inStateVector,
                                            ostream*     inOperator )
{
  __bcierr.SetFlushHandler( BCIError::RuntimeError );
  __bciout.SetFlushHandler( BCIError::Warning );
  _phase = processing;
  _paramlist = inParamList;
  _statelist = inStateList;
  _statevector = inStateVector;
  _operator = inOperator;
  for( ExtensionsContainer::iterator i = Extensions().begin(); i != Extensions().end(); ++i )
    ( *i )->Process();
}

// Called before any call to GenericFilter::StopRun().
void EnvironmentBase::EnterStopRunPhase( PARAMLIST*   inParamList,
                                         STATELIST*   inStateList,
                                         STATEVECTOR* inStateVector,
                                         ostream*     inOperator )
{
  __bcierr.SetFlushHandler( BCIError::RuntimeError );
  __bciout.SetFlushHandler( BCIError::Warning );
  _phase = resting;
  _paramlist = inParamList;
  _statelist = inStateList;
  _statevector = inStateVector;
  _operator = inOperator;
  for( PARAMLIST::iterator i = _paramlist->begin(); i != _paramlist->end(); ++i )
    i->second.Unchanged();
  for( ExtensionsContainer::iterator i = Extensions().begin(); i != Extensions().end(); ++i )
    ( *i )->StopRun();
}

// Called before any call to GenericFilter::Resting().
void EnvironmentBase::EnterRestingPhase( PARAMLIST*   inParamList,
                                         STATELIST*   inStateList,
                                         STATEVECTOR* inStateVector,
                                         ostream*     inOperator )
{
  __bcierr.SetFlushHandler( BCIError::RuntimeError );
  __bciout.SetFlushHandler( BCIError::Warning );
  _phase = resting;
  _paramlist = inParamList;
  _statelist = inStateList;
  _statevector = inStateVector;
  _operator = inOperator;
  for( PARAMLIST::iterator i = _paramlist->begin(); i != _paramlist->end(); ++i )
    i->second.Unchanged();
  for( ExtensionsContainer::iterator i = Extensions().begin(); i != Extensions().end(); ++i )
    ( *i )->Resting();
}

////////////////////////////////////////////////////////////////////////////////
// Environment definitions
////////////////////////////////////////////////////////////////////////////////
Environment::Environment()
{
#if 0
  if( GetPhase() != construction )
    _bcierr << "Environment descendant instantiated "
               "outside construction phase.";
#endif
}


