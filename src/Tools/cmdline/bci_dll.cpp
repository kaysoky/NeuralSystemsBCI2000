////////////////////////////////////////////////////////////////////
// File:        bci_dll.h
// Date:        Jul 12, 2005
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: Provides a framework for dlls that contain BCI2000
//              filters.
////////////////////////////////////////////////////////////////////
#include "bci_dll.h"

#include "UGenericFilter.h"
#include "UEnvironment.h"

#include <iostream>
#include <strstream> // strstreambuf is obsolete but fits our
                     // purposes better than stringbuf because
                     // it allows direct manipulation of the
                     // underlying buffer (which is normally a bad
                     // idea but much more efficient here).

using namespace std;

static ostream sVis( new strstreambuf );
static ostream sOut( new strstreambuf );
ostream sErr( new strstreambuf );

static void
ResetStreams()
{
  dynamic_cast<strstreambuf*>( sVis.rdbuf() )->freeze( false );
  sVis.clear();
  delete sVis.rdbuf( new strstreambuf );

  dynamic_cast<strstreambuf*>( sOut.rdbuf() )->freeze( false );
  sOut.clear();
  delete sOut.rdbuf( new strstreambuf );

  dynamic_cast<strstreambuf*>( sErr.rdbuf() )->freeze( false );
  sErr.clear();
  delete sErr.rdbuf( new strstreambuf );
}

static class FilterWrapper : public Environment
{
 public:
  FilterWrapper() : mpStatevector( NULL ) {}

  const char*
  FilterName()
  {
    const char* name = NULL;
    PARAMLIST paramlist;
    STATELIST statelist;
    Environment::EnterConstructionPhase( &paramlist, &statelist, NULL, NULL );
    GenericFilter::InstantiateFilters();
    GenericFilter* pFilter = GenericFilter::GetFilter<GenericFilter>();
    if( pFilter )
      name = typeid( *pFilter ).name();
    GenericFilter::DisposeFilters();
    Environment::EnterNonaccessPhase();
    if( name == NULL )
    {
      sErr << "Could not create filter instance.\n"
           << "Make sure there is a filter definition with a "
           << "\"RegisterFilter\" statement linked into the executable."
           << endl;
    }
    return name;
  }

  bool
  PutParameter( const char* parameterLine )
  {
    PARAM param;
    if( !param.ReadFromStream( istrstream( parameterLine ) ) )
    {
      sErr << "Error parsing parameter definition line." << endl;
      return false;
    }
    mParameters[ param.GetName() ] = param;
    return true;
  }

  bool
  PutState( const char* stateLine )
  {
    STATE state;
    if( !state.ReadFromStream( istrstream( stateLine ) ) )
    {
      sErr << "Error parsing state definition line." << endl;
      return false;
    }
    delete mpStatevector;
    mpStatevector = NULL;
    mStates.AddState2List( &state );
    return true;
  }

  bool
  SetStateValue( const char* stateName, long value )
  {
    __bcierr.clear();
    State( stateName ) = value;
    return __bcierr.flushes() == 0;
  }

  bool
  GetStateValue( const char* stateName, short& value )
  {
    __bcierr.clear();
    value = State( stateName );
    return __bcierr.flushes() == 0;
  }

  bool
  SetStatevector( const char* statevectorData, long statevectorLength )
  {
    __bcierr.clear();
    if( Statevector()->GetStateVectorLength() != statevectorLength )
    {
      sErr << "Length of input data does not match state vector length." << endl;
      return false;
    }
    ::memcpy( Statevector()->GetStateVectorPtr(), statevectorData, statevectorLength );
    return true;
  }

  bool
  Instantiate()
  {
    __bcierr.clear();
    Environment::EnterConstructionPhase( &mParameters, &mStates, NULL, &sVis );
    GenericFilter::InstantiateFilters();
    bool result = ( __bcierr.flushes() == 0 );
    Environment::EnterNonaccessPhase();
    return result;
  }

  bool
  Dispose()
  {
    __bcierr.clear();
    GenericFilter::DisposeFilters();
    bool result = ( __bcierr.flushes() == 0 );
    Environment::EnterNonaccessPhase();
    return result;
  }

  bool
  Preflight( long& ioChannels, long& ioElements )
  {
    __bcierr.clear();
    mInputProperties = SignalProperties( ioChannels, ioElements, SignalType::float32 );
    Environment::EnterPreflightPhase( &mParameters, &mStates, NULL, &sVis );
    GenericFilter::PreflightFilters( mInputProperties, mOutputProperties );
    bool result = ( __bcierr.flushes() == 0 );
    Environment::EnterNonaccessPhase();
    if( result )
    {
      ioChannels = mOutputProperties.Channels();
      ioElements = mOutputProperties.Elements();
      mInputSignal = GenericSignal( mInputProperties );
      mOutputSignal = GenericSignal( mOutputProperties );
    }
    return result;
  }

  bool
  Initialize()
  {
    __bcierr.clear();
    delete mpStatevector;
    mpStatevector = NULL;
    Environment::EnterInitializationPhase( &mParameters, &mStates, Statevector(), &sVis );
    GenericFilter::InitializeFilters();
    bool result = ( __bcierr.flushes() == 0 );
    Environment::EnterNonaccessPhase();
    return result;
  }

  bool
  StartRun()
  {
    __bcierr.clear();
    Environment::EnterStartRunPhase( &mParameters, &mStates, Statevector(), &sVis );
    GenericFilter::StartRunFilters();
    bool result = ( __bcierr.flushes() == 0 );
    Environment::EnterNonaccessPhase();
    return result;
  }

  bool
  Process( const double* inputSignal, double* outputSignal )
  {
    __bcierr.clear();
    for( size_t element = 0; element < mInputSignal.Elements(); ++element )
      for( size_t channel = 0; channel < mInputSignal.Channels(); ++channel )
        mInputSignal( channel, element ) = *inputSignal++;

    Environment::EnterProcessingPhase( &mParameters, &mStates, Statevector(), &sVis );
    GenericFilter::ProcessFilters( &mInputSignal, &mOutputSignal );
    bool result = ( __bcierr.flushes() == 0 );
    Environment::EnterNonaccessPhase();

    if( result )
      for( size_t element = 0; element < mOutputSignal.Elements(); ++element )
        for( size_t channel = 0; channel < mOutputSignal.Channels(); ++channel )
          *outputSignal++ = mOutputSignal( channel, element );

    return result;
  }

  bool
  StopRun()
  {
    __bcierr.clear();
    Environment::EnterStopRunPhase( &mParameters, &mStates, Statevector(), &sVis );
    GenericFilter::StopRunFilters();
    bool result = ( __bcierr.flushes() == 0 );
    Environment::EnterNonaccessPhase();
    return result;
  }

  bool
  Resting()
  {
    __bcierr.clear();
    Environment::EnterRestingPhase( &mParameters, &mStates, Statevector(), &sVis );
    GenericFilter::RestingFilters();
    bool result = ( __bcierr.flushes() == 0 );
    Environment::EnterNonaccessPhase();
    return result;
  }

  bool Halt()
  {
    __bcierr.clear();
    Environment::EnterStopRunPhase( &mParameters, &mStates, Statevector(), &sVis );
    GenericFilter::HaltFilters();
    bool result = ( __bcierr.flushes() == 0 );
    Environment::EnterNonaccessPhase();
    return result;
  }

 private:
  STATEVECTOR*     Statevector()
  {
    if( !mpStatevector )
      mpStatevector = new STATEVECTOR( &mStates, true );
    return mpStatevector;
  }

  SignalProperties mInputProperties,
                   mOutputProperties;
  GenericSignal    mInputSignal,
                   mOutputSignal;
  PARAMLIST        mParameters;
  STATELIST        mStates;
  STATEVECTOR*     mpStatevector;


} wrapper;

/*
function:  GetInfo
purpose:   Reports filter name and compilation information.
arguments: None.
returns:   Pointer to a null-terminated string holding the information requested.
           The output buffer is allocated inside the DLL, and not meant to be
           deallocated by the caller.
*/
char* DLLEXPORT
GetInfo()
{
  strstreambuf* sb = dynamic_cast<strstreambuf*>( sOut.rdbuf() );
  sb->freeze( false );

  const char* name = wrapper.FilterName();
  if( name == NULL )
    return GetError();

  sOut << "Filter name: " << name << '\n'
       << "BCI2000 filter DLL framework compiled " __DATE__ "\n"
       << ends;
  return sb->str();
}

/*
function:  GetError
purpose:   Retrieve error output from the previously called function.
arguments: None.
returns:   Pointer to a null-terminated string containing error output.
           The output buffer is allocated inside the DLL, and not meant to be
           deallocated by the caller.
*/
char* DLLEXPORT
GetError()
{
  strstreambuf* sb = dynamic_cast<strstreambuf*>( sErr.rdbuf() );
  sb->freeze( false );
  sErr << '\0';
  return sb->str();
}

/*
function:  PutParameter
purpose:   Parses a BCI2000 parameter definition line, and adds the resulting
           parameter object to the filter's parameter list, or changes the value of
           a parameter if it exists.
arguments: Pointer to a NULL terminated parameter line string.
returns:   True (1) if no error occurred.
*/
int DLLEXPORT
PutParameter( char* parameterLine )
{
  ResetStreams();
  return wrapper.PutParameter( parameterLine );
}

/*
function:  PutState
purpose:   Parses a BCI2000 state definition line, and adds the resulting state
           object to the filter's state list.
arguments: Pointer to a NULL terminated state line string.
returns:   True (1) if no error occurred.
*/
int DLLEXPORT
PutState( char* stateLine )
{
  ResetStreams();
  return wrapper.PutState( stateLine );
}

/*
function:  SetStateValue
purpose:   Sets the value of a state to a given value.
arguments: Pointer to a NULL terminated state name string; new state value.
returns:   True (1) if no error occurred.
*/
int DLLEXPORT
SetStateValue( char* stateName, short value )
{
  ResetStreams();
  return wrapper.SetStateValue( stateName, value );
}

/*
function:  GetStateValue
purpose:   Gets the value of a state.
arguments: Pointer to a NULL terminated state name string; pointer to state value.
returns:   True (1) if no error occurred.
*/
int DLLEXPORT
GetStateValue( char* stateName, short* valuePtr )
{
  ResetStreams();
  return wrapper.GetStateValue( stateName, *valuePtr );
}

/*
function:  SetStatevector
purpose:   Sets the DLL's state vector to the binary values contained in a state vector.
arguments: Pointer and length of state vector data. The length must match the length of the
           state vector inside the DLL.
returns:   True (1) if no error occurred.
*/
int DLLEXPORT
SetStatevector( char* statevectorData, long statevectorLength )
{
  ResetStreams();
  return wrapper.SetStatevector( statevectorData, statevectorLength );
}

/*
function:  Instantiate
purpose:   Instantiate the filters contained in the dll.
arguments: n/a
returns:   True (1) if no error occurred.
*/
int DLLEXPORT
Instantiate( void )
{
  ResetStreams();
  return wrapper.Instantiate();
}

/*
function:  Dispose
purpose:   Dispose of all filter instances.
arguments: n/a
returns:   True (1) if no error occurred.
*/
int DLLEXPORT
Dispose( void )
{
  ResetStreams();
  return wrapper.Dispose();
}

/*
function:  Preflight
purpose:   Calls the filters' Preflight() function.
arguments: Dimensions of the input signal, pointers to output signal dimensions.
returns:   True (1) if no error occurred.
*/
int DLLEXPORT
Preflight( long* ioChannels, long* ioElements )
{
  ResetStreams();
  return wrapper.Preflight( *ioChannels, *ioElements );
}

/*
function:  Initialize
purpose:   Calls the filters' Initialize() function.
arguments: n/a
returns:   True (1) if no error occurred.
*/
int DLLEXPORT
Initialize( void )
{
  ResetStreams();
  return wrapper.Initialize();
}

/*
function:  StartRun
purpose:   Calls the filters' StartRun() function.
arguments: n/a
returns:   True (1) if no error occurred.
*/
int DLLEXPORT
StartRun( void )
{
  ResetStreams();
  return wrapper.StartRun();
}

/*
function:  Process
purpose:   Calls the filters' Process() function.
arguments: Pointers to input and output data.
           Input and output arrays are expected to have the dimensions specified/obtained
           using the Preflight() function.
returns:   True (1) if no error occurred.
*/
int DLLEXPORT
Process( double* inputSignal, double* outputSignal )
{
  ResetStreams();
  return wrapper.Process( inputSignal, outputSignal );
}

/*
function:  StopRun
purpose:   Calls the filters' StopRun() function.
arguments: n/a
returns:   True (1) if no error occurred.
*/
int DLLEXPORT
StopRun( void )
{
  ResetStreams();
  return wrapper.StopRun();
}

/*
function:  Resting
purpose:   Calls the filters' Resting() function.
arguments: n/a
returns:   True (1) if no error occurred.
*/
int DLLEXPORT
Resting( void )
{
  ResetStreams();
  return wrapper.Resting();
}

/*
function:  Halt
purpose:   Calls the filters' Halt() function.
arguments: n/a
returns:   True (1) if no error occurred.
*/
int DLLEXPORT
Halt( void )
{
  ResetStreams();
  return wrapper.Halt();
}
