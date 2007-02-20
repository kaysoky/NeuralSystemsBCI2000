////////////////////////////////////////////////////////////////////////////////
// $Id$
//
// File: UEnvironment.cpp
//
// $Log$
// Revision 1.14  2006/01/11 19:08:44  mellinger
// Adaptation to latest revision of parameter and state related class interfaces.
//
// Revision 1.13  2005/12/20 11:42:41  mellinger
// Added CVS id and log to comment.
//
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
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "UEnvironment.h"

#include "MessageHandler.h"
#include "USysCommand.h"
#include "UBCIError.h"
#include "MeasurementUnits.h"
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
    if( Parameters->Exists( name ) )
      param = &( *Parameters )[ name.c_str() ];
    else
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
  else if( Parameters->Exists( name ) )
    param = &( *Parameters )[ name.c_str() ];
  return param;
}

PARAM*
EnvironmentBase::GetOptionalParamPtr( const string& name,
                                      const string& inLabel1,
                                      const string& inLabel2,
                                      int&          outIndex1,
                                      int&          outIndex2 ) const
{
  PARAM* param = GetOptionalParamPtr( name );
  if( param != NULL )
  {
    // If labels are given, they must exist in the parameter.
    // Otherwise, return a NULL pointer.
    if( inLabel1 != "" )
    {
      if( param->LabelsDimension1().Exists( inLabel1 ) )
        outIndex1 = param->LabelsDimension1()[ inLabel1 ];
      else
        param = NULL;
    }
    if( inLabel2 != "" )
    {
      if( param->LabelsDimension2().Exists( inLabel2 ) )
        outIndex2 = param->LabelsDimension2()[ inLabel2 ];
      else
        param = NULL;
    }
  }
  return param;
}

void
EnvironmentBase::CheckRange( const PARAM* param,
                             int row, int column ) const
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
ParamRef
EnvironmentBase::Parameter( const string& name, int row, int column ) const
{
#ifdef TODO
# error Range Check for all values of a parameter if row == column == 0
#endif // TODO
  PARAM* param = GetParamPtr( name );
  if( param != NULL )
      CheckRange( param, row, column );
  return ParamRef( param, row, column );
}

ParamRef
EnvironmentBase::Parameter( const string& name,
                            const string& rowLabel, int column ) const
{
  PARAM* param = GetParamPtr( name );
  size_t row = 0;
  if( param != NULL )
  {
      row = param->LabelsDimension1()[ rowLabel ];
      CheckRange( param, row, column );
  }
  return ParamRef( param, row, column );
}

ParamRef
EnvironmentBase::Parameter( const string& name,
                            int row, const string& columnLabel ) const
{
  PARAM* param = GetParamPtr( name );
  size_t column = 0;
  if( param != NULL )
  {
      column = param->LabelsDimension2()[ columnLabel ];
      CheckRange( param, row, column );
  }
  return ParamRef( param, row, column );
}

ParamRef
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
  return ParamRef( param, row, column );
}

const ParamRef
EnvironmentBase::OptionalParameter( double defaultValue,
                                    const string& name,
                                    int row,
                                    int column ) const
{
  return _OptionalParameter( defaultValue, GetOptionalParamPtr( name ), row, column );
}

const ParamRef
EnvironmentBase::OptionalParameter( double defaultValue,
                                    const string& name,
                                    const string& rowLabel, int column ) const
{
  int row = 0;
  PARAM* param = GetOptionalParamPtr( name, rowLabel, "", row, column );
  return _OptionalParameter( defaultValue, param, row, column );
}

const ParamRef
EnvironmentBase::OptionalParameter( double defaultValue,
                                    const string& name,
                                    int row, const string& columnLabel ) const
{
  int column = 0;
  PARAM* param = GetOptionalParamPtr( name, "", columnLabel, row, column );
  return _OptionalParameter( defaultValue, param, row, column );
}

const ParamRef
EnvironmentBase::OptionalParameter( double defaultValue,
                                    const string& name,
                                    const string& rowLabel, const string& columnLabel ) const
{
  int row = 0,
      column = 0;
  PARAM* param = GetOptionalParamPtr( name, rowLabel, columnLabel, row, column );
  return _OptionalParameter( defaultValue, param, row, column );
}

const ParamRef
EnvironmentBase::OptionalParameter( const string& name,
                                    int row,
                                    int column ) const
{
  return _OptionalParameter( "", GetOptionalParamPtr( name ), row, column );
}

const ParamRef
EnvironmentBase::OptionalParameter( const string& name,
                                    const string& rowLabel, int column ) const
{
  int row = 0;
  PARAM* param = GetOptionalParamPtr( name, rowLabel, "", row, column );
  return _OptionalParameter( "", param, row, column );
}

const ParamRef
EnvironmentBase::OptionalParameter( const string& name,
                                    int row, const string& columnLabel ) const
{
  int column = 0;
  PARAM* param = GetOptionalParamPtr( name, "", columnLabel, row, column );
  return _OptionalParameter( "", param, row, column );
}

const ParamRef
EnvironmentBase::OptionalParameter( const string& name,
                                    const string& rowLabel, const string& columnLabel ) const
{
  int row = 0,
      column = 0;
  PARAM* param = GetOptionalParamPtr( name, rowLabel, columnLabel, row, column );
  return _OptionalParameter( "", param, row, column );
}

const ParamRef
EnvironmentBase::_OptionalParameter( double defaultValue,
                                     PARAM* param,
                                     int row, int column ) const
{
  ostringstream os;
  os << defaultValue;
  return _OptionalParameter( os.str(), param, row, column );
}

const ParamRef
EnvironmentBase::_OptionalParameter( const string& defaultValue,
                                     PARAM* inParam,
                                     int row, int column ) const
{
  static PARAM defaultParam;
  PARAM* param = &defaultParam;
  size_t row_idx = ( row == ParamRef::none ? 0 : row ),
         column_idx = ( column == ParamRef::none ? 0 : column );
  if( inParam == NULL || row_idx >= inParam->GetNumRows() || column_idx >= inParam->GetNumColumns() )
    defaultParam.SetValue( defaultValue, row_idx, column_idx );
  else
  {
    param = inParam;
    CheckRange( param, row_idx, column_idx );
  }
  return ParamRef( param, row, column );
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
StateRef
EnvironmentBase::State( const char* name ) const
{
#ifdef TODO
# error In the type adapter, check whether the value fits into the state.
#endif // TODO
  int location = 0,
      length = 0;
  const STATELIST* statelist = NULL;

  if( _phase != processing )
    statelist = States;
  else if( Statevector != NULL )
    statelist = &Statevector->Statelist();

  if( statelist == NULL )
    _bcierr << "States are inaccessible at this time."
            << endl;
  else
  {
    if( !statelist->Exists( name ) )
      _bcierr << "State \"" << name << "\" is inaccessible." << endl;
    else
    {
      const STATE& state = ( *statelist )[ name ];
      location = state.GetLocation();
      length = state.GetLength();
      if( length < 1 )
        _bcierr << "State \"" << name << "\" has zero length." << endl;
    }
  }
  return StateRef( Statevector, location, length );
}

// Read access to an optional state by its name.
const StateRef
EnvironmentBase::OptionalState( short defaultValue, const char* name ) const
{
  int location = 0,
      length = 0;
  STATEVECTOR* statevector = NULL;
  const STATELIST* statelist = NULL;

  if( _phase != processing )
    statelist = States;
  else if( Statevector != NULL )
    statelist = &Statevector->Statelist();

  if( statelist == NULL )
    _bcierr << "States are inaccessible at this time."
            << endl;
  else if( statelist->Exists( name ) )
  {
    const STATE& state = ( *statelist )[ name ];
    statevector = Statevector;
    location = state.GetLocation();
    length = state.GetLength();
    if( length < 1 )
      _bcierr << "State \"" << name << "\" has zero length." << endl;
  }
  return StateRef( statevector, location, length, defaultValue );
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
        for( size_t i = 0; i < _paramlist->Size(); ++i )
          if( ( *_paramlist )[ i ].Changed() )
            changedParameters.Add( ( *_paramlist )[ i ] );
        if( !changedParameters.Empty() )
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

  double SamplingRate = 1.0,
         SampleBlockSize = 1.0,
         SourceChGain = 1.0;

  if( Parameters != NULL )
  {
    if( Parameters->Exists( "SamplingRate" ) )
      SamplingRate = ::atof( ( *Parameters )[ "SamplingRate" ].GetValue() );
    if( Parameters->Exists( "SampleBlockSize" ) )
      SampleBlockSize = ::atof( ( *Parameters )[ "SampleBlockSize" ].GetValue() );
    if( Parameters->Exists( "SourceChGain" ) )
      SourceChGain = ::atof( ( *Parameters )[ "SourceChGain" ].GetValue( 0 ) );
  }

  MeasurementUnits::InitializeTimeUnit( SamplingRate / SampleBlockSize );
  MeasurementUnits::InitializeFreqUnit( 1.0 / SamplingRate );
  MeasurementUnits::InitializeVoltageUnit( 1e6 / SourceChGain );

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
  for( size_t i = 0; i < _paramlist->Size(); ++i )
    ( *_paramlist )[ i ].Unchanged();
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
  for( size_t i = 0; i < _paramlist->Size(); ++i )
    ( *_paramlist )[ i ].Unchanged();
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


