/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
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

class TTask
{
private:
        float x_pos, y_pos;
        float cursor_x_start, cursor_y_start;
        float ball_x_start, ball_y_start;
        float ball_x_pos, ball_y_pos;
        float ball_delta_x, ball_delta_y;
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

        float CursorSpeed;

        int Ntargets;
        int targetcount;
        int ranflag;
        int targs[NTARGS];
        int Hits;
        int Misses;

        bool HitOrMiss;                          // after trial: true, if hit; false if miss 

        long randseed;

        unsigned short CurrentRunning;          // value of state - internal
        int ItiTime;                            // Counter - internal
        int ItiDuration;                        // Goal    - internal
        int PtpTime;                            // Pretrial Pause
        int PtpDuration;
        float runstarttime;                     // time run begins
        float timelimit;                        // duration of run
        int trial, run;
        float timepassed;

        unsigned short CurrentStimulusTime;
        unsigned short CurrentCursorMovement;

        void ShuffleTargs( int ntargs );
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
        void UpdateCursorPos(int sig_x);
        void TTask::UpdateBallPos();
public:
        void ReadStateValues(STATEVECTOR *);
        void WriteStateValues(STATEVECTOR *);
        void Initialize(PARAMLIST *plist, STATEVECTOR *, CORECOMM *, TApplication *);
        void Process(short * );
        TTask(PARAMLIST *plist, STATELIST *slist);
        ~TTask( void );
} ;

#endif


