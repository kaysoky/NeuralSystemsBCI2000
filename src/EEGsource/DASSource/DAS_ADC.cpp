////////////////////////////////////////////////////////////////////////////////
//
// File: DAS_ADC.cpp
//
// Author: juergen.mellinger@uni-tuebingen.de
//         original version by thilo.hinterberger@uni-tuebingen.de
//
// Date: Sep 18, 2003
//
// Description: A source class that interfaces to the
//              ComputerBoards/Measurement Computing Universal Library.
//
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "DAS_ADC.h"

#include "UBCIError.h"
#include "DASQueue.h"
#include <assert>

using namespace std;

// Register the source class with the framework.
RegisterFilter( TDAS_ADC, 1 );

TDAS_ADC::TDAS_ADC()
{
  BEGIN_PARAMETER_DEFINITIONS
   "Source int SoftwareCh= 8 8 1 16384 // "
       "number of digitized channels",
   "Source int SampleBlockSize= 16 16 1 16384 // "
       "Size of Blocks in Samples",
   "Source int SamplingRate= 256 256 0 0 // "
       "Sampling Rate in Samples per Second",
   "Source int BoardNumber= 0 0 0 0 // "
       "Number of A/D Board as displayed by the InstaCal program",
   "Source floatlist ADRange= { min max } -10 10 0 0 0 // "
       "A/D input range in Volts, e.g. -5 5 or 0 10 (only certain values are supported)",
  END_PARAMETER_DEFINITIONS
}

void
TDAS_ADC::Preflight( const SignalProperties&,
                             SignalProperties& outSignalProperties ) const
{
  // "Owned" parameters (those defined in the constructor) are automatically
  // checked for existence and range.

  // Try to initialize the board with the parameters available.
  DASQueue::DASInfo boardInfo =
  {
    Parameter( "BoardNumber" ),
    Parameter( "SamplingRate" ),
    Parameter( "SampleBlockSize" ),
    Parameter( "SoftwareCh" ),
    Parameter( "ADRange", "min" ), Parameter( "ADRange", "max" ),
  };
  DASQueue preflightQueue;
  preflightQueue.open( boardInfo );
  preflightQueue.close();

  // Input signal checks.
  /* input signal will be ignored */

  // Requested output signal properties.
  outSignalProperties = SignalProperties( boardInfo.numChannels,
                                  boardInfo.sampleBlockSize, SignalType::int16 );

  // Sanity check for TransmitCh, TransmitChList, SoftwareCh.
  // This doesn't really belong here -- rather it belongs into framework code.
  PreflightCondition( Parameter( "SoftwareCh" ) >= Parameter( "TransmitCh" ) );
  PreflightCondition( Parameter( "TransmitChList" )->GetNumValues() == Parameter( "TransmitCh" ) );
  int greatestEntryInTransmitCh = 0; // The name will appear in a possible user message.
  for( size_t i = 0; i < Parameter( "TransmitChList" )->GetNumValues(); ++i )
    if( Parameter( "TransmitChList", i ) > greatestEntryInTransmitCh )
      greatestEntryInTransmitCh = Parameter( "TransmitChList", i );
  PreflightCondition( greatestEntryInTransmitCh <= Parameter( "SoftwareCh" ) );
}

void
TDAS_ADC::Initialize()
{
  DASQueue::DASInfo boardInfo =
  {
    Parameter( "BoardNumber" ),
    Parameter( "SamplingRate" ),
    Parameter( "SampleBlockSize" ),
    Parameter( "SoftwareCh" ),
    Parameter( "ADRange", "min" ), Parameter( "ADRange", "max" ),
  };
  inputQueue.open( boardInfo );
  if( !inputQueue.is_open() )
    bcierr << "Could not initialize A/D board" << endl;
}

void
TDAS_ADC::Process( const GenericSignal*, GenericSignal* outSignal )
{
  for( size_t sample = 0; sample < outSignal->Elements(); ++sample )
    for( size_t channel = 0; channel < outSignal->Channels(); ++channel )
    {
      if( !inputQueue )
      {
        bcierr << "Lost connection to A/D board" << endl;
        return;
      }
      outSignal->SetValue( channel, sample, inputQueue.front() );
      inputQueue.pop();
    }
}

void TDAS_ADC::Halt()
{
  inputQueue.close();
  inputQueue.clear();
}

