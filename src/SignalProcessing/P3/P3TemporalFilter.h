/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------

#ifndef P3TemporalFilterH
#define P3TemporalFilterH
//---------------------------------------------------------------------------

#include "UGenericFilter.h"
#include "UGenericVisualization.h"

#include <map>

#define MAX_ERPBUFFERS          3000
#define ERPBUFCODE_EMPTY        -1

class P3TemporalFilter : public GenericFilter
{
private:
       enum
       {
         noError = 1,
         generalError = 0,
       };
       GenericVisualization *vis;
       GenericSignal    *ERPBufSamples[MAX_ERPBUFFERS], *vissignal;
       bool     visualize;
       void     GetStates();
       float    mindispval, maxdispval;
       int      CurrentRunning, OldRunning, CurrentStimulusCode, OldStimulusCode, CurrentStimulusType;
       int      ERPBufCode[MAX_ERPBUFFERS], ERPBufType[MAX_ERPBUFFERS], ERPBufSampleCount[MAX_ERPBUFFERS];
       bool     ApplyForNewERPBuffer(int StimulusCode, int StimulusType, int numchannels, int numsamples);
       void     DeleteAllERPBuffers();
       void     DeleteERPBuffer(int cur_buf);
       int      maxstimuluscode;
       void     AppendToERPBuffers(const GenericSignal *input);
       int      ProcessERPBuffers(GenericSignal *output);
       STATEVECTOR     *statevector;
       int      numERPsnecessary, targetERPchannel;
       int      mNumberOfSequences;
       std::map<int, bool> mERPDone;

public:
       P3TemporalFilter();
  virtual ~P3TemporalFilter();
  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize();
  virtual void Process(const GenericSignal *Input, GenericSignal *Output);
private:
       int      numsamplesinERP, numchannels;
};
#endif // P3TemporalFilterH




