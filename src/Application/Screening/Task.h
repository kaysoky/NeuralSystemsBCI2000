/*************************************************************************
Task.h is the header file for the Right Justified Boxes task
*************************************************************************/
#ifndef TaskH
#define TaskH

#include <stdio.h>
#include "UBitRate.h"
#include "UCoreComm.h"
#include "UGenericVisualization.h"
#include "UGenericFilter.h"
#include "UBCItime.h"

#define NTARGS  8

class TTask : public GenericFilter
{
private:
        float limit_right;
        float limit_left;
        float limit_top;
        float limit_bottom;
        float TargetWidth;
        float TargetHeight;
        float targx[NTARGS+1];
        float targy[NTARGS+1];
        float targsizex[NTARGS+1];
        float targsizey[NTARGS+1];
        float targy_btm[NTARGS+1];

        int Ntargets;
        int TargetOrien;
        int targetcount;
        int ranflag;
        int targs[NTARGS];

        unsigned short OldRunning;
        unsigned short OldCurrentTarget;

        long randseed;

        unsigned short TargetDuration;           // duration of target on screen (update units)
        int CurrentTargetDuration;
        int BaselineInterval;                    // period for intercept computation
        unsigned short CurrentTarget;            // value of state - internal
        int TargetTime;                          // Counter - internal
        unsigned short CurrentBaseline;
        int BaselineTime;
        unsigned short CurrentFeedback;
        int FeedbackTime;
        unsigned short CurrentIti;              // value of state - internal
        unsigned short CurrentRunning;          // value of state - internal
        unsigned short CurrentRest;             // value of state- internal
        int Resting;
        int ItiTime;                            // Counter - internal
        int ItiDuration;                        // Goal    - internal
        float runstarttime;                     // time run begins
        float timelimit;                        // duration of run
        int trial, run;
        float timepassed;

        unsigned short CurrentStimulusTime;

        void ShuffleTargs( int ntargs );
        void Rest( void );
        void Iti( void );
        void Feedback( short, short );
        void ComputeTargets( int ntargets );
        int GetTargetNo( int ntargs );
        void UpdateDisplays( void );
        void UpdateSummary( void );
        FILE *appl;
        GenericVisualization    *vis;
        BITRATE         bitrate;
        BCITIME         *bcitime;
        TApplication    *Applic;
public:
          TTask();
  virtual ~TTask();
  virtual void Preflight( const SignalProperties&, SignalProperties& ) const {}
  virtual void Initialize();
  virtual void Process( const GenericSignal* Input, GenericSignal* Output );

        void ReadStateValues(STATEVECTOR *);
        void WriteStateValues(STATEVECTOR *);
} ;

#endif
