#include "PCHIncludes.h"
#pragma hdrstop

#include "UFilterHandling.h"

#include "UGenericFilter.h"
#include "UBCIError.h"
#include "MeasurementUnits.h"

#pragma package(smart_init)

FILTERS::FILTERS( void*, void* )
: SignalF( &mOutputBuffer ),
  was_error( false )
{
  BEGIN_PARAMETER_DEFINITIONS
    "Filtering int NumControlSignals= 2"
      " 2 2 2 // the number of transmitted control signals (must be 2)",
  END_PARAMETER_DEFINITIONS

  GenericFilter::InstantiateFilters();
  was_error = ( __bcierr.flushes() > 0 );
}

FILTERS::~FILTERS()
{
  GenericFilter::DisposeFilters();
}

int
FILTERS::Initialize( void*, void*, void* )
{
  MeasurementUnits::InitializeTimeUnit( Parameter( "SamplingRate" ) / Parameter( "SampleBlockSize" ) );
  SignalProperties inputProperties = GenericSignal( Parameter( "TransmitCh" ), Parameter( "SampleBlockSize" ) ),
                   outputProperties = inputProperties;
  GenericFilter::PreflightFilters( inputProperties, outputProperties );
  if( __bcierr.flushes() > 0 )
  {
    was_error = true;
    return 0;
  }
  mInputBuffer.SetProperties( inputProperties );
  mOutputBuffer.SetProperties( outputProperties );
  GenericFilter::InitializeFilters();
  was_error = ( __bcierr.flushes() > 0 );
  return !was_error;
}

int
FILTERS::Resting( void* )
{
  GenericFilter::RestingFilters();
  was_error = ( __bcierr.flushes() > 0 );
  return !was_error;
}

int
FILTERS::Process( const char* inData )
{
  // We don't care about endianness here.
  const short* data = reinterpret_cast<const short*>( inData );
  for( size_t channel = 0; channel < mInputBuffer.Channels(); ++channel )
    for( size_t sample = 0; sample < mInputBuffer.GetNumElements( channel ); ++sample )
      mInputBuffer( channel, sample ) = *data++;
  GenericFilter::ProcessFilters( &mInputBuffer, &mOutputBuffer );
  was_error = ( __bcierr.flushes() > 0 );
  return !was_error;
}


