//---------------------------------------------------------------------------

#ifndef StatFilterH
#define StatFilterH
//---------------------------------------------------------------------------

#include "UGenericFilter.h"
#include "UGenericVisualization.h"
#include "Statistics.h"

class StatFilter : public GenericFilter
{
 private:
       class NormalFilter* nf;
       class ClassFilter* clsf;
       int Ntargets;                    // number of targets
       int InterceptEstMode;            // mode of estimating intercept  0 = none
       float InterceptProportion;       // proportion of intercept used
       float HorizInterceptProp;        // proportion of horizontal intercept used
       int InterceptLength;             // length of running average
       int Trend_Control;               // control of % Correct trend
       int HorizTrend_Control;          // horizontal control of % correct
       int Trend_Win_Lth;               // length of window for trend control
       float LinTrend_Lrn_Rt;           // rate of learning for linear trend control
       float QuadTrend_Lrn_Rt;          // rate of learning for quadratic trend control
       int WtControl;                   // control of Classifier weights
       float WtRate;                    // Learning rate for Classifier control
   //    int LRWtControl;                 // Horizontal wt adaptation
   //    float LRWtRate;                  // Horizontal classifier learning rate
       float ud_intercept;
       float ud_gain;
       float lr_intercept;
       float lr_gain;
       float desiredpix;
       float horizpix;
       int CurrentTarget;
       int CurrentBaseline;
       int CurrentOutcome;
       int CurrentStimulusTime;
       int CurrentFeedback;
       int CurrentIti;
       TRIALSTAT cur_stat;
       TRIALSTAT cur_lr_stat;

       int trend_flag;
       int intercept_flag;
       int weight_flag;
       char FName[128];
       char OName[128];
       FILE *Statfile;
       FILE *Sfile;

       int BaseNum[16];        // MaxNumTargets?
       int BaseHits[16];

       short Xadapt;             // control of classifier weight adaptation
       short Yadapt;
       short AdaptCode;

       STATISTICS *stat;
       bool visualize;
       GenericVisualization *vis;
       GenericSignal *StatSignal;
       void GetStates();
       int GetBaselineHits( void );
       
 public:
          StatFilter();
  virtual ~StatFilter();
  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize();
  virtual void Process( const GenericSignal*, GenericSignal* );
  virtual void Resting();
};
#endif


