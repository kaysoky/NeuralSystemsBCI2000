//---------------------------------------------------------------------------

#ifndef StatFilterH
#define StatFilterH
//---------------------------------------------------------------------------

#include "UGenericFilter.h"
#include "UGenericVisualization.h"
#include "ClassFilter.h"
#include "NormalFilter.h"
#include "Statistics.h"

class StatFilter : public GenericFilter
{
private:
       int instance;
       int Ntargets;                    // number of targets
       int InterceptEstMode;            // mode of estimating intercept  0 = none
       float InterceptProportion;       // proportion of intercept used
       int InterceptLength;             // length of running average
       int Trend_Control;               // control of % Correct trend
       int Trend_Win_Lth;               // length of window for trend control
       float LinTrend_Lrn_Rt;           // rate of learning for linear trend control
       float QuadTrend_Lrn_Rt;          // rate of learning for quadratic trend control
       int WtControl;                   // control of Classifier weights
       float WtRate;                    // Learning rate for Classifier control
       float ud_intercept;
       float ud_gain;
       float lr_intercept;
       float lr_gain;
       float desiredpix;
       int CurrentTarget;
       int CurrentBaseline;
       int CurrentOutcome;
       int CurrentStimulusTime;
       int CurrentFeedback;
       int CurrentIti;
       TRIALSTAT cur_stat;

       int trend_flag;
       int intercept_flag;
       int weight_flag;
       char FName[128];
       char OName[128];
       FILE *Statfile;
       FILE *Sfile;

       int BaseNum[16];        // MaxNumTargets?
       int BaseHits[16];

       STATELIST *statelist;
       PARAMLIST *paramlist;
       STATISTICS *stat;
       bool visualize;
       GenericVisualization *vis;
       GenericSignal *StatSignal;
       void GetStates();
       int GetBaselineHits( void );
public:
       StatFilter(PARAMLIST *plist, STATELIST *slist);
       StatFilter(PARAMLIST *plist, STATELIST *slist, int instance);
  virtual ~StatFilter();
       int Resting( ClassFilter * );
  virtual void Initialize(PARAMLIST *plist, STATEVECTOR *statevector, CORECOMM *);
       int Process(    //	CalibrationFilter *Calf,
		       //	SpatialFilter *Sf,
		       //	TemporalFilter *Tf,
		       	ClassFilter *,
                        GenericSignal *SignalE,
			NormalFilter *nf,
                        GenericSignal *SignalF
                          );
};
#endif


