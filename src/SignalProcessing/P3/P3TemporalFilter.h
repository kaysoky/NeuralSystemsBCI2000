//---------------------------------------------------------------------------

#ifndef TemporalFilterH
#define TemporalFilterH
//---------------------------------------------------------------------------

#include "UGenericFilter.h"
#include "UGenericVisualization.h"

#define MAX_ERPBUFFERS          1500
#define ERPBUFCODE_EMPTY        -1

class P3TemporalFilter : public GenericFilter
{
private:
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
public:
       P3TemporalFilter(PARAMLIST *plist, STATELIST *slist);
       P3TemporalFilter(PARAMLIST *plist, STATELIST *slist, int instance);
  virtual ~P3TemporalFilter();
  virtual void Initialize(PARAMLIST *plist, STATEVECTOR *statevector, CORECOMM *);
  virtual void Process(const GenericSignal *Input, GenericSignal *Output);
       int      numsamplesinERP, numchannels;
};
#endif


