/******************************************************************************
 * $Id$                                                                       *
 * Program:   TMSI.EXE                                                        *
 * Module:    TMSiADC.CPP                                                     *
 * Comment:   Definition for the TMSiADC class                                *
 * Version:   0.04                                                            *
 * Author:    M.M.Span                                                        *
 * Copyright: (C) RUG University of Groningen                                 *
 ******************************************************************************
 * Version History:                                                           *
 *                                                                            *
 * V0.01 - 12/10/2005 - First start                                           *
 * V0.02 - 27/10/2005 - Primary working version: entered testing phase        *
 * V0.03 - 25/01/2006 - Multiple devices (PORTI) useable                      *
 *                      Synchronizing PORTIs work in progress                 *
 * V0.04 - 03/04/2006 - Porti Synchro disbanded. Working on selecting channels*
 * V0.05 - 15/05/2006 - Using the features pull out unused channels from the  *
 *                      common reference pool                                 *
 * $Log$
 * Revision 1.2  2006/07/05 15:20:10  mellinger
 * Minor formatting and naming changes; removed unneeded data members.
 *
 * Revision 1.1  2006/07/04 18:45:50  mellinger
 * Put files into CVS.
 *                                                                      *
 ******************************************************************************/

#include "PCHIncludes.h"
#pragma hdrstop

#include "TMSiADC.h"
#include "UBCIError.h"
#include "UGenericSignal.h"
#include "MeasurementUnits.h"

#include <dos.h>
#include <stdio.h>
#include <math.h>

using namespace std;

RegisterFilter( TMSiADC, 1 );

TMSiADC::TMSiADC()
: mpMaster( NULL ),
  mValuesToRead( 0 ),
  mBufferSize( 0 ),
  mSrate( 0 ),
  mBufferMulti( 4 ),
  mSoftwareCh(19),
  mHardwareCh( 0 ),
  mSampleBlockSize( 0 ),
  mSamplingRate( 0 )
{
  for( size_t i = 0; i < sizeof( mSignalBuffer ) / sizeof( *mSignalBuffer ); ++i )
    mSignalBuffer[ i ] = 0;

  // the values below are echoed into the user interface:
  // Parameters are 'adjustable' in the gui.
  BEGIN_PARAMETER_DEFINITIONS
    "Source intlist SoftwareChList= 19 "
      "1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 "
      "// list of EEG channels",

    "Source int SampleBlockSize= 250 "
      "// this is the number of samples transmitted at a time",
    "Source int SamplingRate= 1000 "
      "// this is the sample rate",
  END_PARAMETER_DEFINITIONS
}

TMSiADC::~TMSiADC()
{
  Halt();
}

void
TMSiADC::Preflight( const SignalProperties&, SignalProperties& outputProperties ) const
{      // Might need a bit of work...

  ULONG BS=mBufferMulti*Parameter("SampleBlockSize");  // in samples: in waitfordata endblock is linked to this...
  ULONG act_rate=1000*Parameter("SamplingRate");       // samplingrate in mHz

  mpMaster->SetSignalBuffer(&act_rate,&BS);

  PreflightCondition( 1000*Parameter("SamplingRate") == act_rate );
  PreflightCondition( Parameter("SamplingRate") > Parameter("SampleBlockSize") );
  outputProperties =  SignalProperties( Parameter("SoftwareCh"),
                      Parameter("SampleBlockSize"),
                      SignalType::int32 );
}

void
TMSiADC::StartDriver()
{
  // Calling functions from the TMSI SDK
  for(ULONG Index=0;Index < MAX_DEVICE;Index++)
          mpDevice[Index] = NULL;

  UseMasterSlave( mpDevice , MAX_DEVICE );

  mpMaster = mpDevice[0];
  if( mpMaster == NULL )
  {
          bcierr <<  "TMSiADC No actual master device is found" << endl;
          return;
  }

  mpMaster->Reset();
  mpMaster->MeasuringMode( MEASURE_MODE_NORMAL  , 1);

  PSIGNAL_FORMAT psf = mpMaster->GetSignalFormat( NULL );

  if( psf != NULL )
  {
      UINT Size = LocalSize( psf );
      if( Size < sizeof( SIGNAL_FORMAT ) * psf->Elements )
      {
          bcierr << "SignalFormat Error" << endl;
          return;
      }

      mHardwareCh = psf->Elements;
      mpMaster->Free( psf );
  }
  else
    bcierr << "Signalformat Error" << endl;
}

void
TMSiADC::Initialize()
{
  mSampleBlockSize   =Parameter( "SampleBlockSize" );
  mSamplingRate      =Parameter( "SamplingRate" );


  mBufferSize     =mBufferMulti*mSampleBlockSize; // in samples!: in waitfordata endblock is linked to this...
  mSrate          =1000*mSamplingRate;           // samplingrate in mHz

  mpMaster->SetSignalBuffer(&mSrate,&mBufferSize);

  mValuesToRead   =mSampleBlockSize * mHardwareCh * 4;   // sizeof type? type dependent? TMS appears to only give 4 byte values.

  if (!mpMaster->Start())
    bcierr << "TMSiADC Initialize returned an error (Device Start)" << endl;
}

void
TMSiADC::Halt()
{
  if (mpMaster->GetDeviceState() == 1)
    mpMaster->Stop(); // if running
}

void
TMSiADC::Process(const GenericSignal*, GenericSignal* outputSignal)
{
  if (WaitForData(&mSignalBuffer[0], mValuesToRead)== TMSIOK)
  {
    int i=0;
    for( unsigned int sample = 0; sample < mSampleBlockSize; ++sample )
        for( unsigned int channel = 0; channel < mHardwareCh; ++channel )
        {
            ( *outputSignal )( channel, sample ) = mSignalBuffer[i++];    // one on one copy: check!
        }
  }
  else
    bcierr << "Error reading data" << endl;
}


//----------------------------------------------------------------------------------------------------
//
//  25/10/05 MMS
//
//  WaitForData:
//
//  Blocking function that fills the SignalBuffer with 'size' samples.
//
//
//
//----------------------------------------------------------------------------------------------------
int
TMSiADC::WaitForData(ULONG* SignalBuffer,ULONG size)
{
  ULONG PercentFull, Overflow;
  static unsigned int mOverflow=0;
  ULONG endblock=100.00/mBufferMulti;
  mpMaster->GetBufferInfo(&Overflow,&PercentFull);

  while( PercentFull < endblock)
  {
          Sleep(0);
          mpMaster->GetBufferInfo(&Overflow,&PercentFull);
  }

  if (Overflow>mOverflow)
  {
          mOverflow=Overflow;
          bciout << "Overflow occurred: " << Overflow  << " % Full= " <<PercentFull << endl;
  }

  ULONG BytesReturned = mpMaster->GetSamples((PULONG)SignalBuffer,size);

  if (BytesReturned!=size)
    bciout << "BytesReturned: "<< BytesReturned <<"; wanted: "<< size <<endl;

  return TMSIOK;
}


//----------------------------------------------------------------------------------------------------
//
//  25/10/05 MMS
//
//  Following function is (almost) verbatim from the TMSi SDK. Needed for driver loading
///----------------------------------------------------------------------------------------------------

ULONG
TMSiADC::UseMasterSlave( RTDeviceEx **Devices , ULONG Max )
{
  ULONG x;

  for(x = 0 ; x < Max ;x++ )
  {
    Devices[x] = new RTDeviceEx(x);
    if( Devices[x] == NULL ) break;

    if( !Devices[x]->InitOk)
    {
        delete Devices[x];
        Devices[x] = NULL;
        break;
    }
  }

  ULONG NrOfDevices = x;

  for( x = 0 ; x < NrOfDevices ; x++ )
  {
    TCHAR DeviceName[40] = _T("Unknown Device");
    ULONG SerialNumber = 0;
    ULONG Size;

    Size = sizeof( SerialNumber );
    RegQueryValueEx( Devices[x]->DeviceRegKey , _T("DeviceSerialNumber"), NULL , NULL , (PBYTE)&SerialNumber , &Size  );

    Size = sizeof( DeviceName );
    RegQueryValueEx( Devices[x]->DeviceRegKey , _T("DeviceDescription"), NULL , NULL , (PBYTE)&DeviceName[0] , &Size  );

    if( x!= 0 )
    {
      HANDLE SlaveHandle = Devices[x]->GetSlaveHandle();
      if(SlaveHandle == 0 )
      {
        bcierr << "Unable to get a handle from device " << (x + 1) << endl;
        break;
      }
      Devices[0]->AddSlave(SlaveHandle);
    }
  }
  return NrOfDevices;
}


