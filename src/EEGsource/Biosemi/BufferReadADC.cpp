#include "PCHIncludes.h"
#pragma hdrstop

#include "BufferReadADC.h"

#include "UGenericSignal.h"
#include <stdio.h>
#include <math.h>
#include "loadsblib.h"

using namespace std;

// Register the source class with the framework.
RegisterFilter( BufferReadADC, 1 );

// **************************************************************************
// Function:   BufferReadADC
// Purpose:    The constructor for the BufferReadADC
//             it fills the provided list of parameters and states
//             with the parameters and states it requests from the operator
// Parameters: plist - the list of parameters
//             slist - the list of states
// Returns:    N/A
// **************************************************************************
BufferReadADC::BufferReadADC()
: samplerate( 0 ),
  blocksize( 0 ),
  channels( 0 ),
  SleepTime( 0 ),
  StartFlag( 0 )
{
  BEGIN_PARAMETER_DEFINITIONS
    "Source int SoftwareCh=       %"
      " 16 1 128 // this is the number of digitized channels",
    "Source int TransmitCh=      32"
      " 32 0 0  // this is the number of transmitted channels",
    "Source int SampleBlockSize= 16"
      " 16 0 0 // this is the number of samples transmitted at a time",
    "Source int SamplingRate=     %"
      " 256 0 0 // this is the sampling rate",
  END_PARAMETER_DEFINITIONS

  // add all states that this ADC requests to the list of states
  BEGIN_STATE_DEFINITIONS
    "Running 1 0 0 0",
    "SourceTime 16 2347 0 0",
  END_STATE_DEFINITIONS
  
  Init_SB_Functions();
  Parameter( "SamplingRate" ) = 1000000L / pSB_GetBinwidthUs();
  Parameter( "SoftwareCh" ) = pSB_GetChRange();
}

BufferReadADC::~BufferReadADC()
{
}

// **************************************************************************
// Function:   Preflight
// Purpose:    Checks parameters for availability and consistence with
//             input signal properties; requests minimally needed properties for
//             the output signal; checks whether resources are available.
// Parameters: Input and output signal properties pointers.
// Returns:    N/A
// **************************************************************************
void BufferReadADC::Preflight( const SignalProperties&,
                                       SignalProperties& outSignalProperties ) const
{
  // Parameter consistency checks: Existence/Ranges and mutual Ranges.

  // Resource availability checks.
  /* The random source does not depend on external resources. */

  // Input signal checks.
  /* The input signal will be ignored. */

  // Requested output signal properties.
  outSignalProperties = SignalProperties(
       Parameter( "SoftwareCh" ), Parameter( "SampleBlockSize" ), SignalType::int16 ); //,
}


// **************************************************************************
// Function:   Initialize
// Purpose:    This function parameterizes the BufferReadADC
//             It is called each time the operator first starts,
//             or suspends and then resumes, the system
//             (i.e., each time the system goes into the main
//             data acquisition loop)
// Parameters: N/A
// Returns:    0 ... on error
//             1 ... no error
// **************************************************************************
void BufferReadADC::Initialize()
{
  // store the value of the needed parameters

 channels = Parameter( "SoftwareCh" );
 blocksize = Parameter( "SampleBlockSize" );
 samplerate = Parameter( "SamplingRate" );

 // ADConfig();

 StartFlag = 0;

}


// **************************************************************************
// Function:   Process
// Purpose:    This function fills the already initialized array RawEEG with
//             values and DOES NOT RETURN, UNTIL ALL DATA IS ACQUIRED
// Parameters: N/A
// Returns:    0 ... on error
//             1 ... no error
// **************************************************************************

#define TAIL_ID 1         // see for interaction with write
void BufferReadADC::Process( const GenericSignal*, GenericSignal* signal )
{
size_t  sample;
size_t  channel;
int     value;
        sample=0;
        char * pSampl;
        Sleep(10);
        MaxSample SampleCopy;
        while (sample<signal->MaxElements())
        {
                if (pSB_pGetItemPchar(&pSampl,TAIL_ID))
                {
                        SampleCopy=  pSampl;
                        pSB_SubtractBl(SampleCopy);
                        AnySampleArray aThis((char *)&SampleCopy);
/*   the old code - all channels shifted up:
                        for (channel=0; channel<signal->Channels(); channel++)
                        {
                                value= aThis[channel]/64;
                                signal->SetValue(channel, sample, (short)value);
                        }
*/
                        for (channel=1; channel<=signal->Channels(); channel++)
                        {
                                value= aThis[channel]/64;
                                signal->SetValue(channel-1, sample, (short)value);
                        }
                        sample++;
                }
        }
}


// **************************************************************************
// Function:   Halt
// Purpose:    This routine shuts down data acquisition
//             In this special case, it does not do anything
// Parameters: N/A
// Returns:    1 ... always
// **************************************************************************
void BufferReadADC::Halt()
{
}



