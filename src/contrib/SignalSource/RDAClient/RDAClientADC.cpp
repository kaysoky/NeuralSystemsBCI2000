////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de,
//         thomas.schreiner@tuebingen.mpg.de
//         jeremy.hill@tuebingen.mpg.de
// Description: A source class that interfaces to the BrainAmp RDA socket
//              interface.
//
// $BEGIN_BCI2000_LICENSE$
// 
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2012: BCI2000 team and many external contributors ]
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

#include "RDAClientADC.h"

#include "GenericSignal.h"
#include "ThreadUtils.h"
#include "BCIStream.h"
#include <string>
#include <sstream>

using namespace std;

// Register the source class with the framework.
RegisterFilter( RDAClientADC, 1 );


RDAClientADC::RDAClientADC()
: mAddMarkerChannel( false )
{
 BEGIN_PARAMETER_DEFINITIONS
  "Source:Signal%20Properties int SourceCh= 32 32 1 %"
          " // the number of digitized and stored channels",
  "Source:Signal%20Properties int SampleBlockSize= 20 20 1 %"
          " // the number of samples transmitted at a time, incoming blocks are always 40ms",
  "Source:Signal%20Properties int SamplingRate= 250 250 1 %"
          " // the sample rate",
  "Source string HostName= localhost"
          " // the name of the host to connect to",
  "Source int AddMarkerChannel= 1 0 0 1 "
          "// duplicate marker data into an additional channel (boolean)",
 END_PARAMETER_DEFINITIONS
 
 BEGIN_STATE_DEFINITIONS
  "RDAMarkers 16 0 0 0",
 END_STATE_DEFINITIONS
}


RDAClientADC::~RDAClientADC()
{
}

void
RDAClientADC::OnPreflight( SignalProperties& Output ) const
{
  // Resource availability and parameter consistency checks.
  RDA::Connection preflightConnection;
  int numSignalChannels = 0,
      numMarkerChannels = 0,
      numOutputChannels = 0;
  preflightConnection.Open( Parameter( "HostName" ) );
  if( preflightConnection )
  {
    const RDA::Info& info = preflightConnection.Info();
    bool goodOffsets = true,
         goodGains   = true;
    numSignalChannels = info.numChannels;
    string matchMessage = " parameter must match the number of channels"
                          " in the recording software";
    if( Parameter( "AddMarkerChannel" ) != 0 )
    {
      numMarkerChannels = 1;
      matchMessage += " plus one marker channel";
    }
    numOutputChannels = numSignalChannels + numMarkerChannels;
    if( Parameter( "SourceCh" ) != numOutputChannels )
      bcierr << "The SourceCh "
             << matchMessage
             << " (" << numOutputChannels << ") ";

    if( Parameter( "SourceChOffset" )->NumValues() != numOutputChannels )
      bcierr << "The number of values in the SourceChOffset"
             << matchMessage
             << " (" << numOutputChannels << ") ";
    else
      for( int i = 0; i < numSignalChannels; ++i )
        goodOffsets &= ( Parameter( "SourceChOffset" )( i ) == 0 );

    if( !goodOffsets )
      bcierr << "The SourceChOffset values for the first "
             << numSignalChannels << " channels "
             << "must be 0";


    if( Parameter( "SourceChGain" )->NumValues() != numOutputChannels )
      bcierr << "The number of values in the SourceChGain"
             << matchMessage
             << " (" << numOutputChannels << ") ";
    else
      for( int i = 0; i < numSignalChannels; ++i )
      {
        double gain = info.channelResolutions[ i ];
        double prmgain = Parameter( "SourceChGain")( i );
        bool same = ( 1e-3 > ::fabs( prmgain - gain ) / ( gain ? gain : 1.0 ) );
        goodGains &= same;

        if ( !same ) bciout << "The RDA server says the gain of"
                            << " channel " << i+1
                            << " is " << gain
                            << " whereas the corresponding value in the"
                            << " SourceChGain parameter is " << prmgain;
      }

    if( !goodGains )
      bcierr << "The SourceChGain values for the first "
             << numSignalChannels << " channels "
             << "must match the channel resolutions settings "
             << "in the recording software";


    if( info.samplingInterval < Limits( info.samplingInterval ).epsilon() )
      bcierr << "The recording software reports an infinite sampling rate "
             << "-- make sure it shows a running signal in its window";
    else
    {
      double sourceSamplingRate = 1e6 / info.samplingInterval;
      if( Parameter( "SamplingRate" ).InHertz() != sourceSamplingRate )
        bcierr << "The SamplingRate parameter must match "
               << "the setting in the recording software "
               << "(" << sourceSamplingRate << ")";

      // Check whether block sizes are sub-optimal.
      int sampleBlockSize = Parameter( "SampleBlockSize" ),
          sourceBlockSize = static_cast<int>( info.blockDuration / info.samplingInterval );
      if( sampleBlockSize % sourceBlockSize != 0 )
        bcierr << "System block size is " << sampleBlockSize << ", must be equal to "
               << "or a multiple of the RDA server's block size, which is " << sourceBlockSize;
    }
  }

  // Requested output signal properties.
#if RDA_FLOAT
  Output = SignalProperties( numOutputChannels + 1, Parameter( "SampleBlockSize" ), SignalType::float32 );
#else
  bciwarn << "You are using the 16 bit variant of the RDA protocol, which is"
          << " considered unreliable. Switching to float is recommended" << endl;
  Output = SignalProperties( numOutputChannels + 1, Parameter( "SampleBlockSize" ), SignalType::int16 );
#endif // RDA_FLOAT
  Output.ChannelLabels()[Output.Channels() - 1] = StateMark + string( "RDAMarkers" );
}


void
RDAClientADC::OnInitialize( const SignalProperties& )
{
  mHostName = Parameter( "HostName" );
  mAddMarkerChannel = ( Parameter( "AddMarkerChannel" ) != 0 );
}

void
RDAClientADC::OnStartAcquisition()
{
  mConnection.Open( mHostName );
}


void
RDAClientADC::OnStopAcquisition()
{
  mConnection.Close();
}

void
RDAClientADC::DoAcquire( GenericSignal& Output )
{
  if( !mConnection.ReceiveData( Output ) )
    Error( "Lost connection to VisionRecorder software" );
  else if( mAddMarkerChannel )
  {
    int from = Output.Channels() - 2,
        to = from + 1;
    for( int el = 0; el < Output.Elements(); ++el )
      Output( to, el ) = Output( from, el );
  }
}
