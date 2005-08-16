////////////////////////////////////////////////////////////////////////////////
//
// File: TransmissionFilter.cpp
//
// Date: Nov 13, 2003
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// Description: A filter that returns a subset of input channels in its output
//              signal.
//
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "TransmissionFilter.h"

#include "defines.h"
#include "UBCIError.h"

using namespace std;

RegisterFilter( TransmissionFilter, 1.1 );

TransmissionFilter::TransmissionFilter()
{
  BEGIN_PARAMETER_DEFINITIONS
    // The TransmitCh parameter is redundant and should be treated as
    // optional.
    "Source int TransmitCh= 4 4 1 128 "
      "// the number of transmitted channels",
    "Source intlist TransmitChList= 4 1 2 3 4 1 1 128 "
      "// list of transmitted channels",
  END_PARAMETER_DEFINITIONS
}

void TransmissionFilter::Preflight( const SignalProperties& InputProperties,
                                          SignalProperties& OutputProperties ) const
{
  // Parameter range checks.
  int numTransmitChannels = Parameter( "TransmitChList" )->GetNumValues();
  if( numTransmitChannels != OptionalParameter( numTransmitChannels, "TransmitCh" ) )
    bcierr << "TransmitCh parameter must match number of entries in TransmitChList" << endl;

  PreflightCondition( Parameter( "SoftwareCh" ) <= InputProperties.Channels() );
  PreflightCondition( Parameter( "SoftwareCh" ) >= Parameter( "TransmitChList" )->GetNumValues() );
  int greatestEntryInTransmitChList = 0, // The names may appear in a user message.
      leastEntryInTransmitChList = numeric_limits<int>::max();
  for( size_t i = 0; i < Parameter( "TransmitChList" )->GetNumValues(); ++i )
  {
    if( Parameter( "TransmitChList", i ) > greatestEntryInTransmitChList )
      greatestEntryInTransmitChList = Parameter( "TransmitChList", i );
    if( Parameter( "TransmitChList", i ) < leastEntryInTransmitChList )
      leastEntryInTransmitChList = Parameter( "TransmitChList", i );
  }
  PreflightCondition( greatestEntryInTransmitChList <= Parameter( "SoftwareCh" ) );
  PreflightCondition( leastEntryInTransmitChList > 0 );

  // Output signal properties.
  OutputProperties = InputProperties;
  OutputProperties.Channels() = Parameter( "TransmitChList" )->GetNumValues();
}

void TransmissionFilter::Initialize2( const SignalProperties& inputProperties,
                                      const SignalProperties& outputProperties )
{
  mChannelList.clear();
  for( size_t i = 0; i < Parameter( "TransmitChList" )->GetNumValues(); ++i )
    mChannelList.push_back( Parameter( "TransmitChList", i ) - 1 );
}

void TransmissionFilter::Process( const GenericSignal* Input,
                                        GenericSignal* Output )
{
  for( size_t i = 0; i < mChannelList.size(); ++i )
    for( size_t j = 0; j < Input->Elements(); ++j )
      ( *Output )( i, j ) = ( *Input )( mChannelList[ i ], j );
}
