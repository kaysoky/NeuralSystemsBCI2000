/******************************************************************************
 * Program:   DTsource.EXE                                                    *
 * Module:    DTADC.CPP                                                       *
 * Comment:   Definition for the Data Translation class                       *
 * Version:   0.01                                                            *
 * Author:    Gerwin Schalk & Dennis McFarland                                *
 * Copyright: (C) Wadsworth Center, NYSDOH                                    *
 ******************************************************************************
 * Version History:                                                           *
 *                                                                            *
 * V0.01 - 05/11/2000 - First start                                           *
 *         05/23/2000 - completed first start and documented                  *
 *         05/24/2000 - Data Translation Driver Added                         *
 *         04/11/2002 - Included error reporting                              *
 ******************************************************************************/
#include "PCHIncludes.h"
#pragma hdrstop

#include "DTADC.h"

#include "UBCIError.h"
#include "UGenericSignal.h"

using namespace std;

DTFUN dtfun;

RegisterFilter( DTADC, 1 );

// **************************************************************************
// Function:   DTADC
// Purpose:    The constructor for the DTADC
//             it fills the provided list of parameters and states
//             with the parameters and states it requests from the operator
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
DTADC::DTADC()
: samplerate( 0 ),
  blocksize( 0 ),
  channels( 0 ),
  SleepTime( 0 ),
  ChanType( 0 ),
  ListSize( 0 ),
  dGain( 0.0 ),
  ClkSource( 0 ),
  dfFreq( 0.0 ),
  Bufferpts( 0 ),
  StartFlag( 0 )
{
 // add all the parameters that this ADC requests to the parameter list
 BEGIN_PARAMETER_DEFINITIONS
    "Source int SoftwareCh=      64 64 1 128 "
        "// this is the number of digitized channels",
    "Source int TransmitCh=      16 5 1 128 "
        "// this is the number of transmitted channels",
    "Source int SampleBlockSize= 16 5 1 128 "
        "// this is the number of samples transmitted at a time",
    "Source int SamplingRate=    128 128 1 4000 "
        "// this is the sample rate",
    "Source string BoardName=    BCI_IN "
        "// this is the name of the AD board",
 END_PARAMETER_DEFINITIONS

 // add all states that this ADC requests to the list of states
 // this is just an example (here, we don't really need all these states)
 BEGIN_STATE_DEFINITIONS
    "Running 1 0 0 0",
    "Active 1 1 0 0",
    "SourceTime 16 2347 0 0",
    "RunActive 1 1 0 0",
 END_STATE_DEFINITIONS
}


DTADC::~DTADC()
{
}

// **************************************************************************
// Function:   ADInit
// Purpose:    This function parameterizes the DTADC
//             It is called each time the operator first starts,
//             or suspends and then resumes, the system
//             (i.e., each time the system goes into the main
//             data acquisition loop (fMain->MainDataAcqLoop())
// Parameters: N/A
// Returns:    0 ... on error
//             1 ... no error
// **************************************************************************
void DTADC::Initialize()
{
 channels = Parameter( "SoftwareCh" );
 blocksize = Parameter( "SampleBlockSize" );
 samplerate = Parameter( "SamplingRate" );
 // ...
 strncpy( dtfun.BoardName, Parameter( "BoardName" ), sizeof( dtfun.BoardName ) );
 dtfun.BoardName[ sizeof( dtfun.BoardName ) - 1 ] = '\0';

 dtfun.InitBoard();
 dtfun.SetFunction( );
 ADConfig();

 StartFlag = 0;
}


int DTADC::ADConfig()
{
        ECODE status;

        ChanType= OL_CHNT_SINGLEENDED;
        ListSize= channels;
        dGain= 2;
        ClkSource= 0;   // 0 means use internal clock
        dfFreq= channels * samplerate;
        Bufferpts= (UINT) channels * blocksize;

        status= dtfun.ConfigAD(  ChanType,
                                ListSize,
                                dGain,
                                ClkSource,
                                dfFreq,
                                Bufferpts );
        return( status );
}

// **************************************************************************
// Function:   Preflight
// Purpose:    Checks parameters for availability and consistence with
//             input signal properties; requests minimally needed properties for
//             the output signal; checks whether resources are available.
// Parameters: Input and output signal properties pointers.
// Returns:    N/A
// **************************************************************************
void DTADC::Preflight( const SignalProperties&,
                             SignalProperties& outSignalProperties ) const
{
  // Constants.
  const size_t signalDepth = 2;

  // Parameter consistency checks: Existence/Ranges and mutual Ranges.
  PreflightCondition( Parameter( "TransmitCh" ) <= Parameter( "SoftwareCh" ) );

  // Resource availability checks.
  strncpy( dtfun.BoardName, Parameter( "BoardName" ), sizeof( dtfun.BoardName ) );
  dtfun.BoardName[ sizeof( dtfun.BoardName ) - 1 ] = '\0';
  dtfun.InitBoard();
  dtfun.SetFunction();
  bool boardAccessible =
    dtfun.ConfigAD(
        OL_CHNT_SINGLEENDED,
        Parameter( "SoftwareCh" ),
        2,
        0,
        Parameter( "SoftwareCh" ) * Parameter( "SamplingRate" ),
        ( UINT )Parameter( "SoftwareCh" ) *  Parameter( "SampleBlockSize" )
      ) == OLSUCCESS
    && dtfun.Start() == OLSUCCESS
    && dtfun.Stop() == OLSUCCESS
    && dtfun.Reset() == OLSUCCESS
    && dtfun.CleanUp() == OLSUCCESS;

  if( !boardAccessible )
    bcierr << "Could not start up data acquisition. Wrong board name?" << endl;

  // Requested output signal properties.
  outSignalProperties = SignalProperties(
       Parameter( "SoftwareCh" ), Parameter( "SampleBlockSize" ), signalDepth );
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
void DTADC::Process( const GenericSignal*, GenericSignal* signal )
{
int     sample, channel, count;
int     value, i, buffersize, time2wait;

 if (StartFlag == 0)
    {
    if (dtfun.Start() == 0)
       {
       return;
       }
    else
       StartFlag= 1;
    }

 // wait until we are notified that data is there
 // let's wait five times longer than what we are supposed to wait
 time2wait=5*(1000*blocksize)/samplerate;
 dtfun.bdone->WaitFor(time2wait);

 // we do not want simultaneous access to dtfun.data[]
 // in case the driver notifies us twice in a row that data is there
 dtfun.data_critsec->Acquire();

 // copy the "oldest" data into our signal
 count= 0;
 for (sample=0; sample<blocksize; sample++)
  {
  for (channel=0; channel<channels; channel++)
   {
   value= dtfun.data[count];
   signal->SetValue( channel, sample, value );
   count++;
   }
  }

 // now, overwrite the "oldest" data by overwriting it with the remaining data blocks
 // in theory, there should always only be one block of data, assuming that the driver
 // notifies us right away when data is there and assuming that the signal happens immediately
 // sometimes, the driver waits longer and then notifies the Callback function (BufferDone) twice
 buffersize=blocksize*channel;   // in samples
 if (dtfun.BufferCount > 1)      // in essence, delete the first buffer by copying the rest over it
    {
    for (sample=0; sample<(dtfun.BufferCount-1)*buffersize; sample++)
     {
     dtfun.data[sample]=dtfun.data[count];
     count++;
     }
    }

 dtfun.BufferCount--;
 dtfun.BufferPtr -= buffersize;

 // if there is more than one data block ready, do not reset the event
 // ADReadDataBlock will then return a second time right away
 if (dtfun.BufferCount == 0) dtfun.bdone->ResetEvent();

 dtfun.data_critsec->Release();
}

void DTADC::Halt()
{
  dtfun.Stop();
  dtfun.Reset();
  dtfun.CleanUp();
}



