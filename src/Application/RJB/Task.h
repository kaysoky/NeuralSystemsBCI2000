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
        float x_pos;
        float y_pos;
        float cursor_x_start;
        float cursor_y_start;
        float limit_right;
        float limit_left;
        float limit_top;
        float limit_bottom;
        float TargetWidth;
        float targx[NTARGS+1];
        float targy[NTARGS+1];
        float targsizex[NTARGS+1];
        float targsizey[NTARGS+1];
        float targy_btm[NTARGS+1];

        int Ntargets;
        int targetcount;
        int ranflag;
        int targs[NTARGS];
        int Hits;
        int Misses;

        unsigned short OldRunning;
        unsigned short OldCurrentTarget;

        bool HitOrMiss;                          // after trial: true, if hit; false if miss 

        long randseed;

        int BaselineInterval;                    // period for intercept computation
        unsigned short CurrentTarget;            // value of state - internal
        int TargetTime;                          // Counter - internal
        int TargetDuration;                      // Goal    - internal
        unsigned short CurrentOutcome;           // value of state - internal
        int OutcomeTime;                         // Counter - internal
        int OutcomeDuration;                     // Goal    - internal
        unsigned short CurrentBaseline;
        int BaselineTime;
        unsigned short CurrentFeedback;
        int FeedbackTime;
        unsigned short CurrentIti;              // value of state - internal
        unsigned short CurrentRunning;          // value of state - internal
        unsigned short CurRunFlag;              // special flag to prevent auto restart
       // int Resting;                            // value of parameter for resting data acquisition period
        unsigned short CurrentRest;             // value of state- internal
        int Resting;
        int ItiTime;                            // Counter - internal
        int ItiDuration;                        // Goal    - internal
        int PtpTime;                            // Pretrial Pause
        int PtpDuration;
        float runstarttime;                     // time run begins
        float timelimit;                        // duration of run
        int trial, run;
        float timepassed;

        unsigned short CurrentStimulusTime;

        void ShuffleTargs( int ntargs );
        void Rest( void );
        void Iti( void );
        void Ptp( void );
        void Feedback( short, short );
        void Outcome( void );
        void TestCursorLocation( float, float );
        void ComputeTargets( int ntargets );
        int GetTargetNo( int ntargs );
        void UpdateDisplays( void );
        void UpdateSummary( void );
        FILE *appl;
        STATEVECTOR             *svect;
        GenericVisualization    *vis;
        CORECOMM        *corecomm;
        BITRATE         bitrate;
        BCITIME         *bcitime;
        TApplication    *Applic;
public:
          TTask( PARAMLIST*, STATELIST* );
  virtual ~TTask();

  virtual void Initialize( PARAMLIST*, STATEVECTOR*, CORECOMM* = NULL );
  virtual void Process( const GenericSignal* Input, GenericSignal* Output );
          void ReadStateValues(STATEVECTOR *);
          void WriteStateValues(STATEVECTOR *);

} ;

#endif
