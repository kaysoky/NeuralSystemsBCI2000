////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A base class for EDF/GDF type output formats.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#ifdef __BORLANDC__
// With inline expansion switched on,
// numeric_limits<long long> causes an internal compiler error.
# pragma option -vi-
#endif // __BORLANDC__

#include "EDFOutputBase.h"

#include "GDF.h"
#include "BCIError.h"

#include <iostream>
#include <sstream>
#include <limits>

using namespace std;


EDFOutputBase::EDFOutputBase()
: mNumRecords( 0 )
{
}


EDFOutputBase::~EDFOutputBase()
{
}


void
EDFOutputBase::Publish() const
{
  BEGIN_PARAMETER_DEFINITIONS
    "Storage string SubjectYearOfBirth= % 1970 % % "
      "// year the subject was born",
    "Storage int SubjectSex= 0 0 0 2 "
      "// 0: not specfied, 1: male, 2: female (enumeration)",
    "Storage string LabID= % % % % "
      "// laboratory identification",
    "Storage string TechnicianID= % % % % "
      "// technician identification",
    "Storage string EquipmentID= BCI2000 % % % "
      "// equipment provider identification",
    "Storage string TransducerType= EEG % % % "
      "// e.g. \"EEG: Ag/AgCl\"",
    "Storage string SignalUnit= uV % % % "
      "// physical unit of calibrated signal",
    "Storage stringlist ChannelNames= 0 % % % % "
      "// channel names",
  END_PARAMETER_DEFINITIONS
}


void
EDFOutputBase::Preflight( const SignalProperties&,
                          const StateVector& ) const
{
  Parameter( "TransducerType" );
  Parameter( "SignalUnit" );
  Parameter( "ChannelNames" );
  if( OptionalParameter( "FilterEnabled", 0 ) == 1 )
  {
    Parameter( "FilterHighPass" );
    Parameter( "FilterLowPass" );
  }
  if( OptionalParameter( "NotchEnabled", 0 ) == 1 )
  {
    Parameter( "NotchHighPass" );
    Parameter( "NotchLowPass" );
  }
  Parameter( "SampleBlockSize" );
  Parameter( "SourceChGain" );
  Parameter( "SourceChOffset" );
}


void
EDFOutputBase::Initialize( const SignalProperties& inProperties,
                           const StateVector& )
{
  mChannels.clear();
  // Enter brain signal channels into the channel list.
  int typeCode = GDF::float64::Code;
  switch( inProperties.Type() )
  {
    case SignalType::int16:
      typeCode = GDF::int16::Code;
      break;
    case SignalType::int32:
      typeCode = GDF::int32::Code;
      break;
    case SignalType::float24:
    case SignalType::float32:
      typeCode = GDF::float32::Code;
      break;
  }
  float digitalMin = inProperties.Type().Min(),
        digitalMax = inProperties.Type().Max();
  // GDF 1.25 uses int64 for digital min and max.
  if( digitalMin < numeric_limits<GDF::int64::ValueType>::min() )
    digitalMin = numeric_limits<GDF::int64::ValueType>::min();
  if( digitalMax > numeric_limits<GDF::int64::ValueType>::max() )
    digitalMax = numeric_limits<GDF::int64::ValueType>::max();
  ChannelInfo channel;
  channel.TransducerType = Parameter( "TransducerType" );
  channel.PhysicalDimension = Parameter( "SignalUnit" );
  channel.SamplesPerRecord = Parameter( "SampleBlockSize" );
  channel.DataType = typeCode;
  channel.DigitalMinimum = digitalMin;
  channel.DigitalMaximum = digitalMax;
  for( int i = 0; i < inProperties.Channels(); ++i )
  {
    if( i < Parameter( "ChannelNames" )->NumValues() )
      channel.Label = Parameter( "ChannelNames" )( i );
    else
    {
      ostringstream oss;
      oss << "Ch" << i + 1;
      channel.Label = oss.str();
    }
    ostringstream filtering;
    if( OptionalParameter( "FilterEnabled", 0 ) == 1 )
      filtering << "HP:" << Parameter( "FilterHighPass" )
                << "LP:" << Parameter( "FilterLowPass" );
    if( OptionalParameter( "NotchEnabled", 0 ) == 1 )
      filtering << "N:"
                << ( Parameter( "NotchHighPass" ) + Parameter( "NotchLowPass" ) ) / 2.0;
    channel.Filtering = filtering.str();
    channel.PhysicalMinimum = Parameter( "SourceChGain" )( i )
                               * ( digitalMin + Parameter( "SourceChOffset" )( i ) );
    channel.PhysicalMaximum = Parameter( "SourceChGain" )( i )
                               * ( digitalMax + Parameter( "SourceChOffset" )( i ) );
    mChannels.push_back( channel );
  }

  // Marker channels to represent states.
  mStateNames.clear();
  ChannelInfo markerChannel;
  markerChannel.TransducerType = "Marker";
  markerChannel.PhysicalDimension = "";
  markerChannel.SamplesPerRecord = Parameter( "SampleBlockSize" );
  markerChannel.DataType = GDF::int16::Code;
  for( int i = 0; i < States->Size(); ++i )
  {
    static string statesToIgnore[] = { "Running", "Recording" };
    bool ignoreCurrentState = false;
    class State& state = ( *States )[ i ];
    for( size_t i = 0; i < sizeof( statesToIgnore ) / sizeof( *statesToIgnore ) && !ignoreCurrentState; ++i )
      ignoreCurrentState |= ( statesToIgnore[ i ] == state.Name() );
    if( !ignoreCurrentState )
    {
      double digitalMinimum = -1,
             digitalMaximum = 1 << state.Length();
      markerChannel.Label = state.Name();
      // DigitalMinimum and DigitalMaximum should be outside the range of actually
      // occurring values.
      markerChannel.DigitalMinimum = digitalMinimum;
      markerChannel.DigitalMaximum = digitalMaximum;
      markerChannel.PhysicalMinimum = digitalMinimum;
      markerChannel.PhysicalMaximum = digitalMaximum;
      Channels().push_back( markerChannel );
      mStateNames.push_back( state.Name() );
    }
  }
}


void
EDFOutputBase::StartRun( ostream& )
{
  mNumRecords = 0;
}


void
EDFOutputBase::StopRun( ostream& )
{
}


template<typename T>
void
EDFOutputBase::PutBlock( ostream& os, const GenericSignal& inSignal, const StateVector& inStatevector )
{
  for( int i = 0; i < inSignal.Channels(); ++i )
    for( int j = 0; j < inSignal.Elements(); ++j )
      GDF::Num<T>( inSignal( i, j ) ).WriteToStream( os );
  for( size_t i = 0; i < mStateNames.size(); ++i )
    for( int j = 0; j < inSignal.Elements(); ++j )
      GDF::PutField< GDF::Num<GDF::int16> >(
        os, inStatevector.StateValue( mStateNames[ i ], min( j, inStatevector.Samples() - 1 ) )
      );
}


void
EDFOutputBase::Write( ostream& os, const GenericSignal& inSignal, const StateVector& inStatevector )
{
  ++mNumRecords;
  switch( inSignal.Type() )
  {
    case SignalType::int16:
      PutBlock<GDF::int16>( os, inSignal, inStatevector );
      break;

    case SignalType::int32:
      PutBlock<GDF::int32>( os, inSignal, inStatevector );
      break;

    case SignalType::float24:
    case SignalType::float32:
      PutBlock<GDF::float32>( os, inSignal, inStatevector );
      break;

    default:
      bcierr << "Unsupported signal data type" << endl;
  }
}
