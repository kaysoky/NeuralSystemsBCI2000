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

#include "RDAClientADC.h"

#include "GenericSignal.h"
#include "BCIError.h"
#include <string>
#include <sstream>

using namespace std;

const float eps = 1e-20f; // Smaller values are considered zero.

// Register the source class with the framework.
RegisterFilter( RDAClientADC, 1 );


RDAClientADC::RDAClientADC()
{
 BEGIN_PARAMETER_DEFINITIONS
    "Source:Signal%20Properties int SourceCh= 33 33 1 %"
            " // the number of digitized and stored channels including marker channel",
    "Source:Signal%20Properties int SampleBlockSize= 20 20 1 %"
            " // the number of samples transmitted at a time, incoming blocks are always 40ms",
    "Source:Signal%20Properties int SamplingRate= 250 250 1 %"
            " // the sample rate",
    "Source string HostName= localhost"
            " // the name of the host to connect to",
 END_PARAMETER_DEFINITIONS
}


RDAClientADC::~RDAClientADC()
{
  Halt();
}

void
RDAClientADC::Preflight( const SignalProperties&,
                               SignalProperties& outSignalProperties ) const
{
  // Resource availability and parameter consistency checks.
  RDAQueue preflightQueue;
  size_t numInputChannels = 0;
  preflightQueue.open( Parameter( "HostName" ).c_str() );
  if( !( preflightQueue && preflightQueue.is_open() ) )
    bcierr << "Cannot establish a connection to the recording software" << endl;
  else
  {
    bool goodOffsets = true,
         goodGains   = true;
    numInputChannels = preflightQueue.info().numChannels + 1;
    const char* matchMessage = " parameter must match the number of channels"
                               " in the recording software plus one";
    if( Parameter( "SourceCh" ) != static_cast<int>( numInputChannels ) )
      bcierr << "The SourceCh "
             << matchMessage
             << " (" << numInputChannels << ") "
             << endl;

    if( Parameter( "SourceChOffset" )->NumValues() != numInputChannels )
      bcierr << "The number of values in the SourceChOffset"
             << matchMessage
             << " (" << numInputChannels << ") "
             << endl;
    else
      for( size_t i = 0; i < numInputChannels - 1; ++i )
        goodOffsets &= ( Parameter( "SourceChOffset" )( i ) == 0 );

    if( !goodOffsets )
      bcierr << "The SourceChOffset values for the first "
             << numInputChannels - 1 << " channels "
             << "must be 0"
             << endl;


    if( Parameter( "SourceChGain" )->NumValues() != numInputChannels )
      bcierr << "The number of values in the SourceChGain"
             << matchMessage
             << " (" << numInputChannels << ") "
             << endl;
    else
      for( size_t i = 0; i < numInputChannels - 1; ++i )
      {
        double gain = preflightQueue.info().channelResolutions[ i ];
        double prmgain = Parameter( "SourceChGain")( i );
        bool same = ( 1e-3 > ::fabs( prmgain - gain ) / ( gain ? gain : 1.0 ) );
        goodGains &= same;

        // bciout<<"channel "<<i+1<<": server="<<gain<<"   prm="<<prmgain<<endl;
        if ( !same ) bciout << "The RDA server says the gain of"
                            << " channel " << i+1
                            << " is " << gain
                            << " whereas the corresponding value in the"
                            << " SourceChGain parameter is " << prmgain << endl;
      }

    if( !goodGains )
      bcierr << "The SourceChGain values for the first "
             << numInputChannels - 1 << " channels "
             << "must match the channel resolutions settings "
             << "in the recording software"
             << endl;


    if( preflightQueue.info().samplingInterval < eps )
      bcierr << "The recording software reports an infinite sampling rate "
             << "-- make sure it shows a running signal in its window"
             << endl;
    else
    {
      double sourceSamplingRate = 1e6 / preflightQueue.info().samplingInterval;
      if( Parameter( "SamplingRate" ).InHertz() != sourceSamplingRate )
        bcierr << "The SamplingRate parameter must match "
               << "the setting in the recording software "
               << "(" << sourceSamplingRate << ")"
               << endl;

      // Check whether block sizes are sub-optimal.
      size_t sampleBlockSize = Parameter( "SampleBlockSize" ),
             sourceBlockSize = static_cast<size_t>( preflightQueue.info().blockDuration / preflightQueue.info().samplingInterval );
      if( sampleBlockSize % sourceBlockSize != 0 && sourceBlockSize % sampleBlockSize != 0 )
        bciout << "Non-integral ratio between source block size (" << sourceBlockSize << ")"
               << " and system block size (" << sampleBlockSize << "). "
               << "This will cause interference jitter."
               << endl;
    }
  }

  // Requested output signal properties.
#if RDA_FLOAT
  outSignalProperties = SignalProperties(
    numInputChannels, Parameter( "SampleBlockSize" ), SignalType::float32 );
#else
  bciout << "You are using the 16 bit variant of the RDA protocol, which is"
         << " considered unreliable. Switching to float is recommended" << endl;
  outSignalProperties = SignalProperties(
    numInputChannels, Parameter( "SampleBlockSize" ), SignalType::int16 );
#endif // RDA_FLOAT
}


void
RDAClientADC::Initialize( const SignalProperties&, const SignalProperties& )
{
  mHostName = string( Parameter( "HostName" ) );

  mInputQueue.clear();
  mInputQueue.open( mHostName.c_str() );
  if( !mInputQueue.is_open() )
    bcierr << "Could not establish connection with recording software"
           << endl;
}


void
RDAClientADC::Process( const GenericSignal&, GenericSignal& Output )
{
  for( int sample = 0; sample < Output.Elements(); ++sample )
    for( int channel = 0; channel < Output.Channels(); ++channel )
    {
      if( !mInputQueue )
      {
        bcierr << "Lost connection to VisionRecorder software" << endl;
        return;
      }
      Output( channel, sample ) = mInputQueue.front();
      mInputQueue.pop();
    }
}


void
RDAClientADC::Halt()
{
  mInputQueue.close();
  mInputQueue.clear();
}


