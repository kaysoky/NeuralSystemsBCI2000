//---------------------------------------------------------------------------

#ifndef UPeakDetectorH
#define UPeakDetectorH
//---------------------------------------------------------------------------

#include "UGenericFilter.h"
#include "UGenericVisualization.h"

class PeakDetector : public GenericFilter
{
private:
       int samples;               // dimension of data matrix
       int channels;              // dimension of data matrix
       int hz;
       int targetchpos;
       int targetchneg;
       float    posthresh, negthresh;
       bool visualize;
       GenericVisualization *vis;
       int get_num_pos_peaks(const GenericSignal *input, int channel);
       int get_num_neg_peaks(const GenericSignal *input, int channel);
public:
       int nBins;
       PeakDetector(PARAMLIST *plist, STATELIST *slist);
  virtual ~PeakDetector();
  virtual void Initialize(PARAMLIST *plist, STATEVECTOR *statevector, CORECOMM *);
  virtual void Process(const GenericSignal *Input, GenericSignal *Output);
};
#endif

