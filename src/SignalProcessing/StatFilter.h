/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------

#ifndef StatFilterH
#define StatFilterH
//---------------------------------------------------------------------------

#include "UGenericFilter.h"
#include "UGenericVisualization.h"
#include "Statistics.h"

#define MaxNumTargets 32

class StatFilter : public GenericFilter
{
 private:
       class NormalFilter* nf;
       class ClassFilter* clsf;
       int Ntargets;                    // number of targets
       int YInterceptEstMode;           // mode of estimating y intercept  0 = none
       int XInterceptEstMode;
       float YMeanProportion;           // proportion of intercept used
       float XMeanProportion;           // proportion of horizontal intercept used
       int SignalWinLth;                // length of running average
       int OutcomeDirection;            // direction of adaption for trial outcome variable- pc vs time
       int Trend_Win_Lth;               // length of window for trend control
       float TrendControlRate;          // rate of learning for linear trend control
       int WtControl;                   // control of Classifier weights
       float WtRate;                    // Learning rate for Classifier control
       float yintercept;
       float ud_gain;
       float xintercept;
       float lr_gain;
       float ypix;
       float horizpix;
       int CurrentTarget;
       int CurrentBaseline;
       int CurrentOutcome;
       int CurrentStimulusTime;
       int CurrentFeedback;
       int CurrentIti;
       TRIALSTAT cur_ystat;
       TRIALSTAT cur_xstat;

       int trend_flag;
       int intercept_flag;
       int weight_flag;
       char FName[1024];
       char OName[1024];
       FILE *Statfile;
       FILE *Sfile;

       int BaseNum[MaxNumTargets];        // MaxNumTargets?
       float BaseHits[MaxNumTargets];

       short Xadapt;             // control of classifier weight adaptation
       short Yadapt;
       short AdaptCode;
       short OutcomeCode;        // state with outcome measure - pc or time?

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




