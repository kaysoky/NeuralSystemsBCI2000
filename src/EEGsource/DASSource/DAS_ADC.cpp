#pragma hdrstop

#include "DAS1402.h"

#include "UBCIError.h"
#include "cbw.h"
#include <stdlib.h>
#include <stdio.h>

using namespace std;

// Register the source class with the framework.
RegisterFilter( TDASSource, 1 );

TDASSource::TDASSource()
: Initialized( false ),
  Gain( BIP5VOLTS ),
  Status( RUNNING ),
  CurCount( 0 ),
  CurIndex( 0 ),
  BufLen( 8192 ),
  Samplerate( 10 ),
  BBeg( 0 ),
  BEnd( 0 ),
  BlockSize( 16 ),
  Channels( 8 ),
  BoardName( new char [ BOARDNAMELEN ] ),
  BoardNum( 0 ),
  ULStat( 0 )
{
 cbGetBoardName( BoardNum, BoardName );
 ULStat = cbStopBackground( BoardNum );

 BEGIN_PARAMETER_DEFINITIONS
   "Source int SoftwareCh= 8 8 1 64 // "
       "number of digitized channels",
   "Source int SampleBlockSize= 16 16 1 1024 // "
       "Size of Blocks in Samples",
   "Source int SamplingRate= 256 256 1 10000 // "
       "Sampling rate in S/s",
   "Source int ADGain= 10 10 1 10 // "
       "Gain of A/D Board",
   "Source intlist SourceChList= 8 0 1 2 3 4 5 6 7 1 0 63 // "
       "Assignment of Source channels",
 END_PARAMETER_DEFINITIONS
}

TDASSource::~TDASSource()
{
  ULStat = cbStopBackground (BoardNum);
  delete [] BoardName;
  if( Initialized )
  {
     free( ADData );
     delete [] RandomData;
  }
}

void TDASSource::Preflight( const SignalProperties&,
                                  SignalProperties& outSignalProperties ) const
{
  // Constants.
  const size_t signalDepth = 2;

  // "Owned" parameters (those defined in the constructor) are automatically
  // checked for existence and range.

  // Parameter values stored for later consideration.
  int sampleBlockSize = Parameter( "SampleBlockSize" ),
      softwareCh = Parameter( "SoftwareCh" ),
      adGain = Parameter( "ADGain" );

  // Parameter consistency checks.
  if( adGain != 5 && adGain != 10 )
    bcierr << "Parameter \"ADGain\" must have a value of either 5 or 10." << endl;

  // Resource availability checks.
  /* TODO: We need to check for board availability here. */

  // Input signal checks.
  /* input signal will be ignored */

  // Requested output signal properties.
  outSignalProperties = SignalProperties( softwareCh, sampleBlockSize, signalDepth );
}

void TDASSource::Initialize()
{
    int ADInitResult;

    ULStat = cbStopBackground (BoardNum);
    Samplerate = Parameter("SamplingRate");
    BlockSize = Parameter("SampleBlockSize");
    Channels = Parameter("SoftwareCh");
    switch( ( int )Parameter("ADGain") )
    {
      case 5:
        Gain = BIP5VOLTS;
        break;
      case 10:
        Gain = BIP10VOLTS;
        break;
      default:
        Gain = BIP5VOLTS;
        bcierr << "Bad value of parameter \"ADGain\"" << endl;
    }

    if (Initialized) {
       free (ADData);
       delete [] RandomData;
    }
    ADData = (WORD *) malloc((BufLen+10)*sizeof(short));
    RandomData = new short [Channels];
    for (int i=0; i<Channels; i++) RandomData[i] = 0;

    cbErrHandling (PRINTALL, DONTSTOP);
    if (StrComp(BoardName,"PCIM-DAS1602/16")==0) Options = BACKGROUND + CONTINUOUS + BURSTMODE + SINGLEIO;
    else {
        if (StrComp(BoardName,"CIO-DAS1402/16")==0) Options = BACKGROUND + CONTINUOUS + BURSTMODE + DMAIO;
        else Options = BACKGROUND + CONTINUOUS + SINGLEIO;
    }
    ULStat = cbAInScan (BoardNum, 0, Channels-1, BufLen, &Samplerate, Gain, ADData, Options);
    if (ULStat==0) ADInitResult=1;
    else ADInitResult=0;
    BBeg = 0;
    BEnd = 0;
    Initialized = true;
#if 0
    return ADInitResult;
#endif
}


int TDASSource::ADDataAvailable()
{
  int Contains;

  ULStat = cbGetStatus (BoardNum, &Status, &CurCount, &CurIndex);
  BEnd = (unsigned int) CurIndex;
  if (BBeg<=BEnd) Contains=BEnd-BBeg;
  else Contains=BufLen-(BBeg-BEnd);
  Contains /= Channels;
  return Contains;
}

void TDASSource::Process( const GenericSignal*, GenericSignal* signal )
{
 int Value;

     while (ADDataAvailable()<BlockSize) {
        Application->ProcessMessages();
        Sleep(10);
     }
     ULStat = cbGetStatus (BoardNum, &Status, &CurCount, &CurIndex);
     BEnd = int(CurIndex);
     if ((BEnd % Channels)!=0) {
         Application->MessageBox("Buffer lost its synchronicity", "A/D-Error", MB_OK);
         ULStat = cbStopBackground (BoardNum);
         BBeg = 0;
         BEnd = 0;
     }
     for (int i=0; i < BlockSize; i++){
        for (int j=0; j < Channels; j++){
           if (BBeg>(BufLen-1)) BBeg=0;
           Value=ADData[BBeg];
           signal->SetValue( j, i, (signed short)(Value-32768) );
           BBeg+=1;
        }
     }
}

#if 0
int TDASSource::ReadRandomDataBlock(GenericIntSignal *SourceSignal)
{
     for (int i=0; i < BlockSize; i++){
        for (int j=0; j < Channels; j++){
           RandomData[j] += (rand() % 201)-100;
           SourceSignal->SetValue( j, i, RandomData[j] );
        }
     }
     Sleep(1000*BlockSize/Samplerate);
     return 1;
}
#endif

void TDASSource::Halt()
{
  ULStat = cbStopBackground( BoardNum );
}



