//---------------------------------------------------------------------------

#ifndef TemporalFilterH
#define TemporalFilterH
//---------------------------------------------------------------------------

#include "UGenericFilter.h"
#include "UGenericVisualization.h"

#define MAX_ERPBUFFERS          50
#define ERPBUFCODE_EMPTY        -1

class P3TemporalFilter : public GenericFilter
{
private:
       GenericVisualization *vis;
       GenericSignal    *ERPBufSamples[MAX_ERPBUFFERS];
       bool     visualize;
       void     GetStates();
       float    mindispval, maxdispval;
       int      CurrentRunning, OldRunning, CurrentStimulusCode, OldStimulusCode, CurrentStimulusType;
       int      ERPBufCode[MAX_ERPBUFFERS], ERPBufType[MAX_ERPBUFFERS], ERPBufSampleCount[MAX_ERPBUFFERS];
       bool     ApplyForNewERPBuffer(int StimulusCode, int StimulusType, int numchannels, int numsamples);
       void     DeleteAllERPBuffers();
       void     DeleteERPBuffer(int cur_buf);
       void     AppendToERPBuffers(GenericSignal *input);
       int      ProcessERPBuffers(GenericSignal *output);
       STATEVECTOR     *statevector;
public:
       P3TemporalFilter(PARAMLIST *plist, STATELIST *slist);
       P3TemporalFilter(PARAMLIST *plist, STATELIST *slist, int instance);
       ~P3TemporalFilter();
       int Initialize(PARAMLIST *plist, STATEVECTOR *statevector, CORECOMM *);
       int Process(GenericSignal *Input, GenericSignal *Output);
       int numsamplesinerp, numchannels, numerpsnecessary;
};
#endif


