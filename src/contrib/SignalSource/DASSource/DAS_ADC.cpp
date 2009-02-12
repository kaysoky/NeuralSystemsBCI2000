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
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "DAS_ADC.h"

#include "BCIError.h"
#include "DASQueue.h"

using namespace std;

// Register the source class with the framework.
RegisterFilter( TDAS_ADC, 1 );

TDAS_ADC::TDAS_ADC()
{
  BEGIN_PARAMETER_DEFINITIONS
   "Source int SourceCh= 8 8 1 % // "
       "number of digitized channels",
   "Source int SampleBlockSize= 16 16 1 % // "
       "Size of Blocks in Samples",
   "Source int SamplingRate= 256 256 % % // "
       "Sampling Rate in Samples per Second",
   "Source int BoardNumber= 0 0 0 0 // "
       "Number of A/D Board as displayed by the InstaCal program",
   "Source floatlist ADRange= { min max } -10 10 0 % % // "
       "A/D input range in Volts, e.g. -5 5 or 0 10 (only certain values are supported)",
  END_PARAMETER_DEFINITIONS
}

void
TDAS_ADC::Preflight( const SignalProperties&, SignalProperties& Output ) const
{
  // "Owned" parameters (those defined in the constructor) are automatically
  // checked for existence and range.

  // Try to initialize the board with the parameters available.
  DASQueue::DASInfo boardInfo =
  {
    Parameter( "BoardNumber" ),
    Parameter( "SamplingRate" ),
    Parameter( "SampleBlockSize" ),
    Parameter( "SourceCh" ),
    Parameter( "ADRange" )( "min" ), Parameter( "ADRange" )( "max" ),
  };
  DASQueue preflightQueue;
  preflightQueue.open( boardInfo );
  preflightQueue.close();

  // Input signal checks.
  /* input signal will be ignored */

  // Requested output signal properties.
  Output = SignalProperties( boardInfo.numChannels,
                             boardInfo.sampleBlockSize, SignalType::int16 );
}

void
TDAS_ADC::Initialize( const SignalProperties&, const SignalProperties& )
{
  DASQueue::DASInfo boardInfo =
  {
    Parameter( "BoardNumber" ),
    Parameter( "SamplingRate" ),
    Parameter( "SampleBlockSize" ),
    Parameter( "SourceCh" ),
    Parameter( "ADRange" )( "min" ), Parameter( "ADRange" )( "max" ),
  };
  mInputQueue.open( boardInfo );
  if( !mInputQueue.is_open() )
    bcierr << "Could not initialize A/D board" << endl;
}

void
TDAS_ADC::Process( const GenericSignal&, GenericSignal& Output )
{
  for( int sample = 0; sample < Output.Elements(); ++sample )
    for( int channel = 0; channel < Output.Channels(); ++channel )
    {
      if( !mInputQueue )
      {
        bcierr << "Lost connection to A/D board" << endl;
        return;
      }
      Output( channel, sample ) = mInputQueue.front();
      mInputQueue.pop();
    }
}

void TDAS_ADC::Halt()
{
  mInputQueue.close();
  mInputQueue.clear();
}

