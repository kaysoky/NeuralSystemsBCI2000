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
 ******************************************************************************/

//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <stdio.h>
#include <math.h>

#include "GenericADC.h"
#include "RandomNumberADC.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)


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
int RandomNumberADC::ADInit()
{
int     i;
bool    flag;
int     channels, blocksize;

 // if we called ADInit() before, we shall delete the stored signal
 // (e.g., we could suspend the system and then resume it - meanwhile, the number
 // of channels could have changed)
 if (signal) delete signal;

 // store the value of the needed parameters
 try
  {
  channels=atoi(paramlist->GetParamPtr("SoftwareCh")->GetValue());
  blocksize=atoi(paramlist->GetParamPtr("SampleBlockSize")->GetValue());
  samplerate=atoi(paramlist->GetParamPtr("SamplingRate")->GetValue());
  sinefrequency=(float)atof(paramlist->GetParamPtr("SineFrequency")->GetValue());
  sineminamplitude=atoi(paramlist->GetParamPtr("SineMinAmplitude")->GetValue());
  sinemaxamplitude=atoi(paramlist->GetParamPtr("SineMaxAmplitude")->GetValue());
  noiseminamplitude=atoi(paramlist->GetParamPtr("NoiseMinAmplitude")->GetValue());
  noisemaxamplitude=atoi(paramlist->GetParamPtr("NoiseMaxAmplitude")->GetValue());
  if (atoi(paramlist->GetParamPtr("ModulateAmplitude")->GetValue()) == 0)
     modulateamplitude=false;
  else
     modulateamplitude=true;
  DCoffset=atoi(paramlist->GetParamPtr("DCoffset")->GetValue());
  sinechannel=atoi(paramlist->GetParamPtr("SineChannel")->GetValue());
  // strcpy(multstate, paramlist->GetParamPtr("MultiplierState")->GetValue());
  }
 catch(...) // if we had an exception here, we did not receive all parameters (why ?)
  { return(0); }

 // create a new GenericIntSignal object that will hold the data for all data blocks
 signal=new GenericIntSignal(channels, blocksize);

 return(1);
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
int RandomNumberADC::ADReadDataBlock()
{
static long count=0;
int     sample, channel, time2wait;
int     sinevalrange, noisevalrange;
TMouse  *cur_mouse;
int     value;
long    longvalue;
double  t;
STATE   *stateptr;
int     stateval, cursorpos;

 // well, we don't want to spit out data continously; thus, wait around 100ms
 time2wait=(int)((float)1000/((float)samplerate/(float)signal->MaxElements))-5;   // just calculate a rough estimate on how long to wait
 Sleep(time2wait);

 sinevalrange=sinemaxamplitude-sineminamplitude;
 noisevalrange=noisemaxamplitude-noiseminamplitude;

 cur_mouse=new TMouse();

 // generate the noisy sine wave and write it into the signal
 for (sample=0; sample<signal->MaxElements; sample++)
  {
  cursorpos=cur_mouse->CursorPos.y/70+1;
  for (channel=0; channel<signal->Channels; channel++)
   {
   t=(double)(count+sample)/(double)samplerate*(double)sinefrequency;
   value=0;
   // create the signal on all channels, or on the one channel selected
   if ((sinechannel == 0) || (sinechannel == channel+1))
      {
      value=(int)((sin(t*2*3.14159265)/2+0.5)*(double)sinevalrange+(double)sineminamplitude);
      if (noisevalrange > 1)
         value+=(int)(rand() % noisevalrange + (int)noiseminamplitude);
      if (modulateamplitude)
         value=(int)((float)value/(float)cursorpos);
      }
   value+=DCoffset;
   signal->SetValue(channel, sample, (short)value);
   }
  }

 delete cur_mouse;
 count=count+signal->MaxElements;
 return(1);
}


// **************************************************************************
// Function:   ADShutdown
// Purpose:    This routine shuts down data acquisition
//             In this special case, it does not do anything (since
//             the random number generator does not have to be turned off)
// Parameters: N/A
// Returns:    1 ... always
// **************************************************************************
int RandomNumberADC::ADShutdown()
{
 return(1);
}


// **************************************************************************
// Function:   RandomNumberADC
// Purpose:    The constructor for the RandomNumberADC
//             it fills the provided list of parameters and states
//             with the parameters and states it requests from the operator
// Parameters: plist - the list of parameters
//             slist - the list of states
// Returns:    N/A
// **************************************************************************
RandomNumberADC::RandomNumberADC(PARAMLIST *plist, STATELIST *slist)
{
FILE            *fp;
char            line[512];

 signal=NULL;

 // store the pointer to the parameter list and state list
 // we need the lists later on, e.g., in ADInit()
 // of course, we then can't meanwhile destroy the paramlist object
 paramlist=plist;
 statelist=slist;

 // add all the parameters that this ADC requests to the parameter list
 strcpy(line, "Source int SoftwareCh=      16 16 1 128        // the number of digitized and stored channels\n");
 paramlist->AddParameter2List(line, strlen(line));
 strcpy(line, "Source int SampleBlockSize= 32 5 1 128         // the number of samples transmitted at a time\n");
 paramlist->AddParameter2List(line, strlen(line));
 strcpy(line, "Source int ModulateAmplitude= 0 0 0 1          // modulate the amplitude with the mouse (0=no, 1=yes)\n");
 paramlist->AddParameter2List(line, strlen(line));
 strcpy(line, "Source int SamplingRate=    256 128 1 4000     // the sample rate\n");
 paramlist->AddParameter2List(line, strlen(line));
 strcpy(line, "Source int SineChannel=    0 0 0 128           // channel number of sinewave (0=all)\n");
 paramlist->AddParameter2List(line, strlen(line));
 strcpy(line, "Source float SineFrequency=    10 10 0 100           // frequency of the sine wave\n");
 paramlist->AddParameter2List(line, strlen(line));
 strcpy(line, "Source int SineMinAmplitude=    -10000 0 -32767 32767           // the minimal output value for sine\n");
 paramlist->AddParameter2List(line, strlen(line));
 strcpy(line, "Source int SineMaxAmplitude=    10000 20000 -32767 32767   // the maximum output value for sine\n");
 paramlist->AddParameter2List(line, strlen(line));
 strcpy(line, "Source int NoiseMinAmplitude=    -3000 0 -32767 32767           // the minimal output value for noise\n");
 paramlist->AddParameter2List(line, strlen(line));
 strcpy(line, "Source int NoiseMaxAmplitude=    3000 3000 -32767 32767   // the maximum output value for noise\n");
 paramlist->AddParameter2List(line, strlen(line));
 strcpy(line, "Source int DCoffset=   0 0 -32767 32767   // DC offset (common to all channels)\n");
 paramlist->AddParameter2List(line, strlen(line));
 //strcpy(line, "Source string MultiplierState=   -1 -1 0 0   // State to use as signal multiplier (-1 == don't use multiplier)\n");
 //paramlist->AddParameter2List(line, strlen(line));

 // add all states that this ADC requests to the list of states
 // this is just an example (here, we don't really need all these states)
 statelist->AddState2List("Running 1 0 0 0\n");
 statelist->AddState2List("SourceTime 16 2347 0 0\n");
}


RandomNumberADC::~RandomNumberADC()
{
 if (signal) delete signal;
 signal=NULL;
}

