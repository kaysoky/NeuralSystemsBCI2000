/******************************************************************************
 * Program:   EEGsource.EXE                                                   *
 * Module:    RandomNumberADC.CPP                                             *
 * Comment:   Definition for the GenericADC class                             *
 * Version:   0.05                                                            *
 * Author:    Gerwin Schalk                                                   *
 * Copyright: (C) Wadsworth Center, NYSDOH                                    *
 ******************************************************************************
 * Version History:                                                           *
 *                                                                            *
 * V0.01 - 05/11/2000 - First start                                           *
 *         05/23/2000 - completed first start and documented                  *
 * V0.02 - 05/26/2000 - changed **RawEEG to *GenericIntSignal                 *
 * V0.03 - 07/25/2000 - made it a waveform generator                          *
 * V0.04 - 03/23/2001 - made the waveform generator more powerful             *
 * V0.05 - 09/28/2001 - can modulate samples based on mouse                   *
 * V0.06 - 03/28/2003 - juergen.mellinger@uni-tuebingen.de:                   *
 *                      now derived from GenericFilter                        *
 ******************************************************************************/
#include "PCHIncludes.h"
#pragma hdrstop

#include "RandomNumberADC.h"

#include "UGenericSignal.h"
#include <stdio.h>
#include <math.h>

using namespace std;

// Register the source class with the framework.
RegisterFilter( RandomNumberADC, 1 );

// **************************************************************************
// Function:   RandomNumberADC
// Purpose:    The constructor for the RandomNumberADC
//             it fills the provided list of parameters and states
//             with the parameters and states it requests from the operator
// Parameters: plist - the list of parameters
//             slist - the list of states
// Returns:    N/A
// **************************************************************************
RandomNumberADC::RandomNumberADC()
: samplerate( 0 ),
  sineminamplitude( 0 ), sinemaxamplitude( 0 ),
  noiseminamplitude( 0 ), noisemaxamplitude( 0 ),
  sinefrequency( 0 ),
  DCoffset( 0 ),
  sinechannel( 0 ),
  sinechannelx( 0 ),
  modulateamplitude( 0 )
{
 mLasttime = -1;
 
 // add all the parameters that this ADC requests to the parameter list
 BEGIN_PARAMETER_DEFINITIONS
   "Source int SoftwareCh=      16 16 1 128 "
       "// the number of digitized and stored channels",
   "Source int SampleBlockSize= 32 5 1 128 "
       "// the number of samples transmitted at a time",
   "Source int ModulateAmplitude= 0 0 0 1 "
       "// modulate the amplitude with the mouse (0=no, 1=yes)",
   "Source int SamplingRate=    256 128 1 4000 "
       "// the sample rate",
   "Source int SineChannel=    0 0 0 128 "
       "// channel number of sinewave for y (0=all)",
   "Source int SineChannelX=    0 0 0 128 "
       "// channel number of sinewave for x",
   "Source float SineFrequency=    10 10 0 100 "
       "// frequency of the sine wave",
   "Source int SineMinAmplitude=    -10000 0 -32767 32767 "
       "// the minimal output value for sine",
   "Source int SineMaxAmplitude=    10000 20000 -32767 32767 "
       "// the maximum output value for sine",
   "Source int NoiseMinAmplitude=    -3000 0 -32767 32767 "
       "// the minimal output value for noise",
   "Source int NoiseMaxAmplitude=    3000 3000 -32767 32767 "
       "// the maximum output value for noise",
   "Source int DCoffset=             0 0 -32767 32767 "
       "// DC offset (common to all channels)",
 //"Source string MultiplierState=   -1 -1 0 0 "
 //    "// State to use as signal multiplier (-1 == don't use multiplier)",
 END_PARAMETER_DEFINITIONS

 // add all states that this ADC requests to the list of states
 // this is just an example (here, we don't really need all these states)
 BEGIN_STATE_DEFINITIONS
   "Running 1 0 0 0",
   "SourceTime 16 2347 0 0",
 END_STATE_DEFINITIONS
}

RandomNumberADC::~RandomNumberADC()
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
void RandomNumberADC::Preflight( const SignalProperties&,
                                       SignalProperties& outSignalProperties ) const
{
  // Constants.
  const size_t signalDepth = 2;

  // Parameter consistency checks: Existence/Ranges and mutual Ranges.
  PreflightCondition( Parameter( "SineMinAmplitude" ) <= Parameter( "SineMaxAmplitude" ) );
  PreflightCondition( Parameter( "NoiseMinAmplitude" ) <= Parameter( "NoiseMaxAmplitude" ) );

  // Resource availability checks.
  /* The random source does not depend on external resources. */

  // Input signal checks.
  /* The input signal will be ignored. */

  // Requested output signal properties.
  outSignalProperties = SignalProperties(
       Parameter( "SoftwareCh" ), Parameter( "SampleBlockSize" ), signalDepth );
}


// **************************************************************************
// Function:   ADInit
// Purpose:    This function parameterizes the RandomNumberADC
//             It is called each time the operator first starts,
//             or suspends and then resumes, the system
//             (i.e., each time the system goes into the main
//             data acquisition loop (fMain->MainDataAcqLoop())
// Parameters: N/A
// Returns:    0 ... on error
//             1 ... no error
// **************************************************************************
void RandomNumberADC::Initialize()
{
  // store the value of the needed parameters
  samplerate = Parameter( "SamplingRate" );
  sinefrequency = Parameter( "SineFrequency" );
  sineminamplitude = Parameter( "SineMinAmplitude" );
  sinemaxamplitude = Parameter( "SineMaxAmplitude" );
  noiseminamplitude = Parameter( "NoiseMinAmplitude" );
  noisemaxamplitude = Parameter( "NoiseMaxAmplitude" );
  modulateamplitude = ( ( int )Parameter( "ModulateAmplitude" ) != 0 );
  DCoffset = Parameter( "DCoffset" );
  sinechannel = Parameter( "SineChannel" );
  sinechannelx = Parameter( "SineChannelX" );
  mLasttime = -1;
}


// **************************************************************************
// Function:   ADReadDataBlock
// Purpose:    This function is called within fMain->MainDataAcqLoop()
//             it fills the already initialized array RawEEG with values
//             and DOES NOT RETURN, UNTIL ALL DATA IS ACQUIRED
// Parameters: N/A
// Returns:    0 ... on error
//             1 ... no error
// **************************************************************************
void RandomNumberADC::Process( const GenericSignal*, GenericSignal* signal )
{
static long count=0;
size_t  sample;
size_t  channel;
int     time2wait;
int     sinevalrange, noisevalrange;
int     value, noise;
long    longvalue;
double  t;
STATE   *stateptr;
int     stateval, cursorpos, cursorposx;

 // well, we don't want to spit out data continously; thus, wait around 100ms
 float sr = samplerate;
 if( sr < 1.0 )
   sr = 1.0;
 time2wait = 1e3 * signal->MaxElements() / sr - 5.0;
 ::Sleep(time2wait);

 sinevalrange=sinemaxamplitude-sineminamplitude;
 noisevalrange=noisemaxamplitude-noiseminamplitude;

// generate the noisy sine wave and write it into the signal
 for (sample=0; sample<signal->MaxElements(); sample++)
  {
  cursorpos=Mouse->CursorPos.y/70+1;
  cursorposx=(Screen->Width-Mouse->CursorPos.x)/70+1;
  for (channel=0; channel<signal->Channels(); channel++)
   {
   t=(double)(count+sample)/(double)samplerate*(double)sinefrequency;
   value=0;
   // create the signal (for Y) on all channels, or on the one channel selected
   if ((sinechannel == 0) || (sinechannel == channel+1))
      {
      if( sinefrequency == 0 && modulateamplitude )
        value = ( sinevalrange * ( Screen->Height / 2 - Mouse->CursorPos.y ) ) / Screen->Height + sineminamplitude;
      else
        value=(int)((sin(t*2*3.14159265)/2+0.5)*(double)sinevalrange+(double)sineminamplitude);
      if (sinefrequency != 0 && modulateamplitude)
         value=(int)((float)value/(float)cursorpos);
   }
   // create the signal on the channel selected for X
   if (sinechannelx == channel+1)
      {
      if( sinefrequency == 0 && modulateamplitude )
        value = ( sinevalrange * ( Screen->Width / 2 - Mouse->CursorPos.x ) ) / Screen->Width + sineminamplitude;
      else
        value=(int)((sin(t*2*3.14159265)/2+0.5)*(double)sinevalrange+(double)sineminamplitude);
      if (sinefrequency != 0 && modulateamplitude)
         value=(int)((float)value/(float)cursorposx);
      }
   if (noisevalrange > 1)
     noise=(int)(rand() % noisevalrange + (int)noiseminamplitude);
   value+= noise;         // add noise after modulating sine wave
   value+=DCoffset;
   const maxvalue = 1 << 15 - 1,
         minvalue = - 1 << 15;
   if( value > maxvalue )
     value = maxvalue;
   if( value < minvalue )
     value = minvalue;
   signal->SetValue(channel, sample, (short)value);
   }
  }

 count=count+signal->MaxElements();
}


// **************************************************************************
// Function:   ADShutdown
// Purpose:    This routine shuts down data acquisition
//             In this special case, it does not do anything (since
//             the random number generator does not have to be turned off)
// Parameters: N/A
// Returns:    1 ... always
// **************************************************************************
void RandomNumberADC::Halt()
{
}



