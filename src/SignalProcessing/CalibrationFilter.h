//---------------------------------------------------------------------------

#ifndef CalibrationFilterH
#define CalibrationFilterH
//---------------------------------------------------------------------------

#include "UGenericFilter.h"

class CalibrationFilter : public GenericFilter
{
private:
       enum
       {
#undef MAXCHAN
         MAXCHAN = 72,
       };
       bool     align;
       bool     visualize;
       float    *offset, *gain;
       int      recordedChans;
       int      transmittedChans;
       float    delta;
       float    w1[MAXCHAN];
       float    w2[MAXCHAN];
       float    old[MAXCHAN];
       class GenericVisualization *vis;
public:
          CalibrationFilter();
  virtual ~CalibrationFilter();
  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize();
  virtual void Process(const GenericSignal *Input, GenericSignal *Output);
};
#endif


