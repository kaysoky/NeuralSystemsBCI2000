//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include <stdlib.h>
#include <stdio.h>
#include "cbw.h"

#include "GenericADC.h"
#include "DAS1402.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)

TDASSource::TDASSource(PARAMLIST *my_paramlist, STATELIST *statelist) : signal( NULL )
{
 char line[255];
 BoardName = new char [BOARDNAMELEN];

 paramlist=my_paramlist;
 
 BoardNum = 0;
 cbGetBoardName(BoardNum, BoardName);
 ULStat = cbStopBackground (BoardNum);
 Gain = BIP5VOLTS;
 Status = RUNNING;
 CurCount = 0;
 CurIndex = 0;
 BufLen = 8192;
 Samplerate = 10;
 BBeg = 0;
 BEnd = 0;
 BlockSize = 16;
 Channels = 8;
 // source variables
    strcpy(line, "Source int SoftwareCh= 8 8 1 64 // number of digitized channels");
    paramlist->AddParameter2List(line, strlen(line));
    strcpy(line, "Source int SampleBlockSize= 16 16 1 1024 // Size of Blocks in Samples");
    paramlist->AddParameter2List(line, strlen(line));
    strcpy(line, "Source int SamplingRate= 256 256 1 10000 // sampling rate in S/s");
    paramlist->AddParameter2List(line, strlen(line));
    strcpy(line, "Source int ADGain= 10 10 1 10 // Gain of A/D Board");
    paramlist->AddParameter2List(line, strlen(line));
    strcpy(line, "Source intlist SourceChList= 8 0 1 2 3 4 5 6 7 1 0 63 // Assignment of Source channels");
    paramlist->AddParameter2List(line, strlen(line));

 Initialized = false;
}

TDASSource::~TDASSource()
{
    ULStat = cbStopBackground (BoardNum);
    delete [] BoardName;
    if (Initialized) {
       free (ADData);
       delete [] RandomData;
    }
    if( signal ) delete signal;
    Initialized=false;
}

int TDASSource::ADShutdown()
{
  ULStat = cbStopBackground (BoardNum);
  return (1);
}


int TDASSource::ADInit()
{
    int ADInitResult;

    ULStat = cbStopBackground (BoardNum);
    if (paramlist->GetParamPtr("SamplingRate")) Samplerate = atoi(paramlist->GetParamPtr("SamplingRate")->GetValue());
    else Samplerate = 256;
    if (paramlist->GetParamPtr("SampleBlockSize")) BlockSize = atoi(paramlist->GetParamPtr("SampleBlockSize")->GetValue());
    else BlockSize = 16;
    if (paramlist->GetParamPtr("SoftwareCh")) Channels = atoi(paramlist->GetParamPtr("SoftwareCh")->GetValue());
    else Channels = 8;
    if (paramlist->GetParamPtr("ADGain")) {
        int AuxGain = atoi(paramlist->GetParamPtr("ADGain")->GetValue());
        if (AuxGain==10) Gain = BIP10VOLTS;
        if (AuxGain==5) Gain = BIP5VOLTS;
    }
    else Gain = BIP5VOLTS;
    if (Initialized) {
       free (ADData);
       delete [] RandomData;
    }
    ADData = (WORD *) malloc((BufLen+10)*sizeof(short));
    RandomData = new short [Channels];
    for (int i=0; i<Channels; i++) RandomData[i] = 0;
    if( signal ) delete signal;
    signal= new GenericIntSignal( Channels, BlockSize );

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
    return ADInitResult;
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

int TDASSource::ADReadDataBlock()
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
           signal->Value[j][i]=(signed short)(Value-32768);
           BBeg+=1;
        }
     }
     return (ULStat==0);
}

int TDASSource::ReadRandomDataBlock(GenericIntSignal *SourceSignal)
{
     for (int i=0; i < BlockSize; i++){
        for (int j=0; j < Channels; j++){
           RandomData[j] += (rand() % 201)-100;
           SourceSignal->Value[j][i]=RandomData[j];
        }
     }
     Sleep(1000*BlockSize/Samplerate);
     return 1;
}
