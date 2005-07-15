////////////////////////////////////////////////////////////////////
// File:    bci_filterwrapper.cpp
// Date:    Jul 12, 2005
// Author:  juergen.mellinger@uni-tuebingen.de
// Description: A BCI2000 filter wrapper that reads a BCI2000
//          compliant binary stream from an input stream, applies
//          a BCI2000 filter, and writes its output to an
//          output stream as a BCI2000 compliant binary stream.
////////////////////////////////////////////////////////////////////
#include "bci_filterwrapper.h"

#include "shared/UGenericFilter.h"
#include "shared/UEnvironment.h"
#include "shared/UGenericVisualization.h"

using namespace std;

const char*
FilterWrapper::FilterName()
{
  const char* pName = NULL;
  PARAMLIST paramlist;
  STATELIST statelist;
  Environment::EnterConstructionPhase( &paramlist, &statelist, NULL, NULL );
  GenericFilter::InstantiateFilters();
  Environment::EnterNonaccessPhase();
  GenericFilter* pFilter = GenericFilter::GetFilter<GenericFilter>();
  if( pFilter == NULL )
    __bcierr << "Could not create filter instance.\n"
             << "Make sure there is a filter definition with a "
             << "\"RegisterFilter\" statement linked into the executable."
             << endl;
  else
    pName = typeid( *pFilter ).name();
  return pName;
}

void
FilterWrapper::FinishProcessing()
{
  if( Environment::GetPhase() == Environment::processing )
    StopRun();
}

bool
FilterWrapper::HandlePARAM( istream& arIn )
{
  PARAM p;
  p.ReadBinary( arIn );
  if( arIn )
    mParamlist[ p.GetName() ] = p;
  return true;
}

bool
FilterWrapper::HandleSTATE( istream& arIn )
{
  if( mpStatevector != NULL )
  {
    if( Environment::GetPhase() == Environment::processing )
      StopRun();
    delete mpStatevector;
    mpStatevector = NULL;
  }
  STATE s;
  s.ReadBinary( arIn );
  if( arIn )
    mStatelist.AddState2List( &s );
  return true;
}

bool
FilterWrapper::HandleSTATEVECTOR( istream& arIn )
{
  if( mpStatevector == NULL )
    mpStatevector = new STATEVECTOR( &mStatelist, true );
  mpStatevector->ReadBinary( arIn );
  if( !mpStatevector->GetStateValue( "Running" )
      && Environment::GetPhase() == Environment::processing )
    StopRun();
  return true;
}

bool
FilterWrapper::HandleVisSignal( istream& arIn )
{
  VisSignal s;
  if( s.ReadBinary( arIn ) && s.GetSourceID() == 0 )
  {
    const GenericSignal& inputSignal = s;
    SignalProperties outputProperties;
    switch( Environment::GetPhase() )
    {
      case Environment::nonaccess:
        {
          GenericFilter::DisposeFilters();

          PARAMLIST filterParams;
          STATELIST filterStates;
          Environment::EnterConstructionPhase( &filterParams, &filterStates, NULL, &mrOperator );
          GenericFilter::InstantiateFilters();
          if( __bcierr.flushes() > 0 )
          {
            arIn.setstate( ios::failbit );
            break;
          }
          // Add the filter's parameters with their default values to the parameter
          // list as far as they are missing from the input stream.
          for( PARAMLIST::iterator i = filterParams.begin(); i != filterParams.end(); ++i )
            if( mParamlist.find( i->second.GetName() ) == mParamlist.end() )
              mParamlist[ i->second.GetName() ] = i->second;
          // If there are no states in the input stream, use the filter's states.
          if( mStatelist.GetNumStates() == 0 )
            mStatelist = filterStates;
        }
        /* no break */
      case Environment::construction:
        if( mpStatevector == NULL )
          mpStatevector = new STATEVECTOR( &mStatelist, true );
        for( int i = 0; i < mStatelist.GetNumStates(); ++i )
          PutMessage( mrOut, *mStatelist.GetStatePtr( i ) );
        Environment::EnterPreflightPhase( &mParamlist, &mStatelist, mpStatevector, &mrOperator );
        GenericFilter::PreflightFilters( inputSignal.GetProperties(), outputProperties );
        mOutputSignal.SetProperties( outputProperties );
        if( __bcierr.flushes() > 0 )
        {
          arIn.setstate( ios::failbit );
          break;
        }
        /* no break */
      case Environment::preflight:
        Environment::EnterInitializationPhase( &mParamlist, &mStatelist, mpStatevector, &mrOperator );
        GenericFilter::InitializeFilters();
        for( PARAMLIST::const_iterator i = mParamlist.begin(); i != mParamlist.end(); ++i )
          PutMessage( mrOut, i->second );
        if( __bcierr.flushes() > 0 )
        {
          arIn.setstate( ios::failbit );
          break;
        }
        /* no break */
      case Environment::initialization:
      case Environment::resting:
        /* no break */
        Environment::EnterStartRunPhase( &mParamlist, &mStatelist, mpStatevector, &mrOperator );
        GenericFilter::StartRunFilters();
        Environment::EnterProcessingPhase( &mParamlist, &mStatelist, mpStatevector, &mrOperator );
        /* no break */
      case Environment::processing:
        {
          bool wasRunning = mpStatevector->GetStateValue( "Running" );
          GenericFilter::ProcessFilters( &inputSignal, &mOutputSignal );
          if( __bcierr.flushes() > 0 )
          {
            arIn.setstate( ios::failbit );
            break;
          }
          bool isRunning = mpStatevector->GetStateValue( "Running" );
          if( isRunning || wasRunning )
            if( mpStatevector->GetStateVectorLength() > 0 )
              PutMessage( mrOut, *mpStatevector );
          if( isRunning )
            PutMessage( mrOut, mOutputSignal );
        }
        break;
      default:
        bcierr << "Unknown Environment phase" << endl;
        arIn.setstate( ios::failbit );
    }
  }
  return arIn;
}

void
FilterWrapper::StopRun()
{
  Environment::EnterStopRunPhase( &mParamlist, &mStatelist, mpStatevector, &mrOut );
  GenericFilter::StopRunFilters();
  Environment::EnterNonaccessPhase();
  Environment::EnterRestingPhase( &mParamlist, &mStatelist, mpStatevector, &mrOut );
  GenericFilter::RestingFilters();
  Environment::EnterNonaccessPhase();
  Environment::EnterRestingPhase( &mParamlist, &mStatelist, mpStatevector, &mrOperator );
}

