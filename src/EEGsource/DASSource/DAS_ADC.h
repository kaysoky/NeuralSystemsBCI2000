//---------------------------------------------------------------------------
#ifndef DAS1402H
#define DAS1402H

#include "UParameter.h"
#include "UState.h"
#include "UGenericSignal.h"

#define ADCERR_NOERR    0
#define ADCERR_INIT     1
#define ADCERR_ADREAD   2

class TDASSource : public GenericADC
{
protected:
        int BlockSize;
        int Channels;
        long Samplerate;
    int BoardNum;
    int ULStat;
    int Gain;
    short Status;
    long CurCount;
    long CurIndex;
    long BufLen;
    unsigned Options;
    unsigned int BBeg;
    unsigned int BEnd;
    int SamplesLeft;
    bool Initialized;
    WORD *ADData;
    short *RandomData;
    char *BoardName;
    PARAMLIST *paramlist;
public:
        TDASSource(PARAMLIST *paramlist, STATELIST *statelist);
        ~TDASSource();
        int ADInit();
        int ADReadDataBlock();
        int ADDataAvailable();
        int     ADShutdown();
        int ReadRandomDataBlock(GenericIntSignal *SourceSignal);
        GenericIntSignal *signal;
 };

//---------------------------------------------------------------------------
#endif
