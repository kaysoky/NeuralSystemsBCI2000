////////////////////////////////////////////////////////////////////////////////
//
// File: UEnvironment.cpp
//
// Description: A mix-in base class that channels access to enviroment-like
//              global objects of types CORECOMM, PARAMLIST, STATELIST,
//              STATEVECTOR, and provides convenient accessor functions
//              and checking utilities.
//
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "UEnvironment.h"

#include "UBCIError.h"
#include <typeinfo>

// The #pragma makes the linker evaluate dependencies for startup initalization.
#pragma package(smart_init)

using namespace std;

// Environment class definitions.
#undef _paramlist
#undef _statelist
#undef _statevector
#undef _corecomm
#undef _phase
PARAMLIST*   Environment::_paramlist = NULL;
STATELIST*   Environment::_statelist = NULL;
STATEVECTOR* Environment::_statevector = NULL;
CORECOMM*    Environment::_corecomm = NULL;
Environment::executionPhase Environment::_phase = Environment::nonaccess;

Environment::paramlistAccessor   Environment::Parameters;
Environment::statelistAccessor   Environment::States;
Environment::statevectorAccessor Environment::Statevector;
Environment::corecommAccessor    Environment::Corecomm;

Environment::Environment()
{
  if( _phase != construction )
    _bcierr << "Environment descendant instantiated "
               "outside construction phase.";
}

void
Environment::CheckRange( const PARAM* param,
                         size_t row, size_t column ) const
{
#ifdef TODO
# error Ensure reasonable ranges and backward compatibility before enabling the range check.
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
Environment::Parameter( const char* name,
                        size_t row, size_t column ) const
{
#ifdef TODO
# error Range Check for all values of a parameter if row == column == 0
# error Check for Preflight access before Initialize/Process access
#endif // TODO
  PARAM* param = NULL;

  if( Parameters == NULL )
    _bcierr << "Tried parameter access during non-access phase."
            << endl;
  else
  {
    param = Parameters->GetParamPtr( name );
    if( param == NULL )
      _bcierr << "Parameter \"" << name << "\" is inaccessible."
              << endl;
    else
      CheckRange( param, row, column );
  }
  return PARAM::type_adapter( param, row, column );
}

const PARAM::type_adapter
Environment::OptionalParameter( double defaultValue,
                                const char* name,
                                size_t row, size_t column ) const
{
  ostringstream os;
  os << defaultValue;
  return OptionalParameter( os.str().c_str(), name, row, column );
}

const PARAM::type_adapter
Environment::OptionalParameter( const char* defaultValue,
                                const char* name,
                                size_t row, size_t column ) const
{
  static PARAM defaultParam;
  PARAM* param = NULL;

  if( Parameters == NULL )
    _bcierr << "Tried parameter access in non-access phase."
            << endl;
  else
  {
    param = Parameters->GetParamPtr( name );
    if( param == NULL )
    {
      defaultParam.SetValue( defaultValue, row, column );
      param = &defaultParam;
    }
    else
      CheckRange( param, row, column );
  }
  return PARAM::type_adapter( param, row, column );
}


bool
Environment::_PreflightCondition( const char* inConditionString,
                                         bool inConditionValue ) const
{
  if( !inConditionValue )
    _bcierr << "Required parameter condition not fulfilled: "
            << inConditionString << endl;
  return inConditionValue;
}


// Read/write access to a state by its name.
STATEVECTOR::type_adapter
Environment::State( const char* name ) const
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
    }
  }
  return STATEVECTOR::type_adapter( Statevector, byteLocation, bitLocation, length );
}

// Read access to an optional state by its name.
const STATEVECTOR::type_adapter
Environment::OptionalState( short defaultValue, const char* name ) const
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
    }
  }
  return STATEVECTOR::type_adapter( statevector, byteLocation, bitLocation, length, defaultValue );
}

// Called to prevent access.
void Environment::EnterNonaccessPhase()
{
  __bcierr.SetFlushHandler( BCIError::LogicError );
  __bciout.SetFlushHandler( BCIError::Warning );
  _phase = nonaccess;
  _paramlist = NULL;
  _statelist = NULL;
  _statevector = NULL;
  _corecomm = NULL;
}
// Called from the framework before any Environment descendant class
// is instantiated.
void Environment::EnterConstructionPhase( PARAMLIST*   inParamList,
                                          STATELIST*   inStateList,
                                          STATEVECTOR* inStateVector,
                                          CORECOMM*    inCoreComm )
{
  __bcierr.SetFlushHandler( BCIError::LogicError );
  __bciout.SetFlushHandler( BCIError::Warning );
  _phase = construction;
  _paramlist = inParamList;
  _statelist = inStateList;
  _statevector = inStateVector;
  _corecomm = inCoreComm;
}

// Called before any call to GenericFilter::Preflight().
void Environment::EnterPreflightPhase( PARAMLIST*   inParamList,
                                       STATELIST*   inStateList,
                                       STATEVECTOR* inStateVector,
                                       CORECOMM*    inCoreComm )
{
  __bcierr.SetFlushHandler( BCIError::ConfigurationError );
  __bciout.SetFlushHandler( BCIError::Warning );
  _phase = preflight;
  _paramlist = inParamList;
  _statelist = inStateList;
  _statevector = NULL;
  _corecomm = inCoreComm;
}

// Called before any call to GenericFilter::Initialize().
void Environment::EnterInitializationPhase( PARAMLIST*   inParamList,
                                            STATELIST*   inStateList,
                                            STATEVECTOR* inStateVector,
                                            CORECOMM*    inCoreComm )
{
  __bcierr.SetFlushHandler( BCIError::RuntimeError );
  __bciout.SetFlushHandler( BCIError::Warning );
  _phase = initialization;
  _paramlist = inParamList;
  _statelist = inStateList;
  _statevector = inStateVector;
  _corecomm = inCoreComm;
}

// Called before any call to GenericFilter::Process().
void Environment::EnterProcessingPhase( PARAMLIST*   inParamList,
                                        STATELIST*   inStateList,
                                        STATEVECTOR* inStateVector,
                                        CORECOMM*    inCoreComm )
{
  __bcierr.SetFlushHandler( BCIError::RuntimeError );
  __bciout.SetFlushHandler( BCIError::Warning );
  _phase = processing;
  _paramlist = inParamList;
  _statelist = inStateList;
  _statevector = inStateVector;
  _corecomm = inCoreComm;
}
