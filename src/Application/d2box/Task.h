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

#include <mmsystem.h>

#define jhalf 32767
#define NTARGS  8

class TTask : public GenericFilter
{
private:
        int useJoy;
        int targetInclude;
        float joy_xgain;
        float joy_ygain;
        float x_pos;
        float y_pos;
        float cursor_x_start;
        float cursor_y_start;
        float CursorStartX;
        float CursorStartY;
        float limit_right;
        float limit_left;
        float limit_top;
        float limit_bottom;
        float size_right;
        float size_left;
        float size_top;
        float size_bottom;
    //    float TargetWidth;
    //    float TargetHeight;
        float targx[NTARGS+1];
        float targy[NTARGS+1];
        float targsizex[NTARGS+1];
        float targsizey[NTARGS+1];
        float targx_rt[NTARGS+1];
        float targy_btm[NTARGS+1];
        short targx_adapt[NTARGS+1];
        short targy_adapt[NTARGS+1];
        short targ_adaptcode[NTARGS+1];

        int Ntargets;
        int targetcount;
        int ranflag;
        int targs[NTARGS];
        int Hits;
        int Misses;

        int jx_cntr;
        int jy_cntr;
        int n_tmat;
        int m_tmat;
        int tmat[7][NTARGS];

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
        int FeedbackDuration;
     //   int FeedbackTime;

        int ntrials;                            // variables for trial time recording
        int ntimeouts;
        double totalfeedbacktime;
        double feedstart;
        int feedflag;

        unsigned short CurrentStimulusTime;

        short CurrentXadapt;
        short CurrentYadapt;
        unsigned CurrentAdaptCode;

        int TestTarget( float, float, int );
        void GetJoy( void );
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
        TTask();
        virtual ~TTask();
        
        void ReadStateValues(STATEVECTOR *);
        void WriteStateValues(STATEVECTOR *);
        virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
        virtual void Initialize();
        virtual void Process( const GenericSignal * Input, GenericSignal * Output );

} ;

#endif
