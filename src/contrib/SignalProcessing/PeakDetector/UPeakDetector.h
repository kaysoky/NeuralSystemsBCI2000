/* (C) 2000-2010, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------

#ifndef UPeakDetectorH
#define UPeakDetectorH
//---------------------------------------------------------------------------

#include "GenericFilter.h"
#include "GenericVisualization.h"

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
       int nBins;

public:
       PeakDetector();
  virtual ~PeakDetector();
  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize( const SignalProperties&, const SignalProperties& );
  virtual void Process(const GenericSignal& Input, GenericSignal& Output);
};
#endif

