//---------------------------------------------------------------------------
#ifndef DAS1402H
#define DAS1402H

#include "UGenericSignal.h"
#include "GenericADC.h"

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

 public:
  TDASSource();
  virtual ~TDASSource();
  virtual void Initialize();
  virtual void Preflight( const SignalProperties&,
                                SignalProperties& ) const;
  virtual void Process(   const GenericSignal*,
                                GenericSignal* );
  virtual void Halt();

  int ADDataAvailable();
//int ReadRandomDataBlock(GenericIntSignal *SourceSignal);
};

//---------------------------------------------------------------------------
#endif
