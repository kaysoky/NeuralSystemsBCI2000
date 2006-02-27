////////////////////////////////////////////////////////////////////////////////
//
// File: RDAClientADC.cpp
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// Date: Jan 3, 2003
//
// Description: A source class that interfaces to the BrainAmp RDA socket
//              interface.
//
// Changes:     Apr 3, 2003: Adaptations to the changes introduced by the error
//              handling facilities.
//
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "RDAClientADC.h"

#include "UGenericSignal.h"
#include "UBCIError.h"
#include <string>
#include <sstream>

using namespace std;

const float eps = 1e-20; // Smaller values are considered zero.

// Register the source class with the framework.
RegisterFilter( RDAClientADC, 1 );

void RDAClientADC::Preflight( const SignalProperties&,
                                    SignalProperties& outSignalProperties ) const
{
  // Resource availability and parameter consistency checks.
  RDAQueue preflightQueue;
  size_t numInputChannels = 0;
  preflightQueue.open( Parameter( "HostName" ) );
  if( !( preflightQueue && preflightQueue.is_open() ) )
    bcierr << "Cannot establish a connection to the recording software" << endl;
  else
  {
    numInputChannels = preflightQueue.info().numChannels + 1;
    const char* matchMessage = " parameter must equal the number of channels"
                               " in the recording software plus one";
    if( Parameter( "SoftwareCh" ) != numInputChannels )
      bcierr << "The SoftwareCh "
             << matchMessage
             << " (" << numInputChannels << ") "
             << endl;
    if( Parameter( "SourceChOffset" )->GetNumValues() != numInputChannels )
      bcierr << "The number of values in the SourceChOffset"
             << matchMessage
             << " (" << numInputChannels << ") "
             << endl;
    if( Parameter( "SourceChGain" )->GetNumValues() != numInputChannels )
      bcierr << "The number of values in the SourceChGain"
             << matchMessage
             << " (" << numInputChannels << ") "
             << endl;

    bool goodOffsets = true,
         goodGains   = true;
    for( size_t i = 0; i < numInputChannels - 1; ++i )
    {
      goodOffsets &=
        ( Parameter( "SourceChOffset", i ) == 0 );
        
      double gain = preflightQueue.info().channelResolutions[ i ];
      goodGains &= (
        gain != 0
        &&
        1e-3 > ::fabs( Parameter( "SourceChGain", i ) - gain ) / gain
      );
    }
    if( !goodOffsets )
      bcierr << "The SourceChOffset values for the first "
             << numInputChannels - 1 << " channels "
             << "must be 0"
             << endl;
    if( !goodGains )
      bcierr << "The SourceChGain values for the first "
             << numInputChannels - 1 << " channels "
             << "must match the channel resolutions settings "
             << "in the recording software ("
             << preflightQueue.info().channelResolutions[ 0 ]
             << ")"
             << endl;

    if( preflightQueue.info().samplingInterval < eps )
      bcierr << "The recording software reports an infinite sampling rate "
             << "-- make sure it shows a running signal in its window"
             << endl;
    else
    {
      float sourceSamplingRate = 1e6 / preflightQueue.info().samplingInterval;
      if( Parameter( "SamplingRate" ) != sourceSamplingRate )
        bcierr << "The SamplingRate parameter must match "
               << "the setting in the recording software "
               << "(" << sourceSamplingRate << ")"
               << endl;

      // Check whether block sizes are sub-optimal.
      size_t sampleBlockSize = Parameter( "SampleBlockSize" ),
             sourceBlockSize =
        preflightQueue.info().blockDuration / preflightQueue.info().samplingInterval;
      if( sampleBlockSize % sourceBlockSize != 0 && sourceBlockSize % sampleBlockSize != 0 )
        bciout << "Non-integral ratio in source and system block sizes. "
               << "This will cause interference jitter"
               << endl;
    }
  }

  // Requested output signal properties.
  outSignalProperties = SignalProperties(
       numInputChannels, Parameter( "SampleBlockSize" ), SignalType::int16 );
}

// **************************************************************************
// Function:   ADInit
// Purpose:    This function initializes the RDAClientADC.
//             It is called each time the operator first starts,
//             or suspends and then resumes, the system
//             (i.e., each time the system goes into the main
//             data acquisition loop (fMain->MainDataAcqLoop())
// Parameters: N/A
// Returns:    Error value
// **************************************************************************
void RDAClientADC::Initialize()
{
  hostName = string( Parameter( "HostName" ) );

  inputQueue.clear();
  inputQueue.open( hostName.c_str() );
  if( !inputQueue.is_open() )
    bcierr << "Could not establish connection with recording software"
           << endl;
}


// **************************************************************************
// Function:   Process
// Purpose:    This function is called within fMain->MainDataAcqLoop().
//             It fills its argument signal with values
//             and does not return until a full block of data is acquired.
// Parameters: Pointer to a signal to be filled with values.
// Returns:    N/A
// **************************************************************************
void RDAClientADC::Process( const GenericSignal*, GenericSignal* SourceSignal )
{
  for( size_t sample = 0; sample < SourceSignal->Elements(); ++sample )
    for( size_t channel = 0; channel < SourceSignal->Channels(); ++channel )
    {
      if( !inputQueue )
      {
        bcierr << "Lost connection to VisionRecorder software" << endl;
        return;
      }
      SourceSignal->SetValue( channel, sample, inputQueue.front() );
      inputQueue.pop();
    }
}

// **************************************************************************
// Function:   Halt
// Purpose:    This routine shuts down data acquisition
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void RDAClientADC::Halt()
{
  inputQueue.close();
  inputQueue.clear();
}

// **************************************************************************
// Function:   RDAClientADC
// Purpose:    The RDAClientADC constructor.
//             It fills the provided list of parameters and states
//             with the parameters and states it requests from the operator.
// Parameters: plist - the list of parameters
//             slist - the list of states
// Returns:    N/A
// **************************************************************************
RDAClientADC::RDAClientADC()
{
 BEGIN_PARAMETER_DEFINITIONS
    "Source int SoftwareCh= 33 33 1 129"
            " // the number of digitized and stored channels including marker channel",
    "Source int SampleBlockSize= 20 20 1 128"
            " // the number of samples transmitted at a time, incoming blocks are always 40ms",
    "Source int SamplingRate= 250 250 1 4000"
            " // the sample rate",
    "Source string HostName= localhost"
            " // the name of the host to connect to",
 END_PARAMETER_DEFINITIONS

  // This should go into the framework code.
 BEGIN_STATE_DEFINITIONS
    "SourceTime 16 0 0 0",
 END_STATE_DEFINITIONS
}

// **************************************************************************
// Function:   ~RDAClientADC
// Purpose:    The RDAClientADC destructor.
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
RDAClientADC::~RDAClientADC()
{
  Halt();
}

