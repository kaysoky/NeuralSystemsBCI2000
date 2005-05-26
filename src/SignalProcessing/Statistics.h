#ifndef StatisticsH
#define StatisticsH

#include <stdio.h>

#define MAX_CONTROLSIG          8
#define MAX_BLSTATES            32
#define MAX_SAMPLESPERTRIAL     500
#define MAX_TRIALS              128
#define MAXDIM                  2          // max dimensions

class TRIALSTAT
{
public:
        float   Intercept;
        float   StdDev;
        float   TargetPC[MAX_BLSTATES];
        int     NumT;
        float   lin;
        float   quad;
        float   aper;
        float   pix;
        int     trial_flag;
        int     update_flag;
};

class CIRCBUF
{
private:
        float   *trialdata[MAX_TRIALS];
        int     trialnum[MAX_TRIALS];
        int     trialsamples[MAX_TRIALS];
public:
        int     trials;
        int     maxtrials;

        CIRCBUF::CIRCBUF();
        CIRCBUF::~CIRCBUF();
        void    PushVal(float val);
        void    NextTrial();
        void    NewTrial(int newtrial);
        void    DeleteTrial(int newtrial);
        float   CalculateAllTrialAverage();
        float   CalculateAllTrialStdDev();
        int     GetAllTrialSampleNum();
};


class STATISTICS
{
private:
        int sign;
        CIRCBUF *circbuf[MAX_CONTROLSIG][MAX_BLSTATES];
        CIRCBUF *targbuf[MAX_BLSTATES];
        int targuse[MAXDIM][MAX_BLSTATES];
        int targval[MAXDIM][MAX_BLSTATES];
        int targval2[MAXDIM][MAX_BLSTATES];
        float   CurAvg[MAX_CONTROLSIG];
        float   CurStdDev[MAX_CONTROLSIG];
        float   current_yintercept;
        float   current_xintercept;
        int     outcome_type;
        FILE    *sfile;
        float   sig_mean[MAX_CONTROLSIG];
        int  oldfeedback[MAX_CONTROLSIG];
        float wt_buf[MAX_CONTROLSIG][128];
        int  use_flag[MAX_CONTROLSIG];
        int oldBLstate[MAXDIM];
        float quadval[MAX_BLSTATES];
public:
        STATISTICS::STATISTICS();
        STATISTICS::~STATISTICS();
        void    SetTrendControl( int bufno, float val, int ntimes );            // seed the trend control running average
        void    SetOutcomeDirection( int dir );                                 //
        void    ProcRunningAvg(int, int, float, TRIALSTAT *);                   // do whole proceedure
        void    ProcTrendControl(int,int, int, int, int, short, TRIALSTAT *, float, int);      // do whole proceedure
        void    ProcWeightControl( int, int, int, int, float *, float *, float, int );   // weight control
        void    SetNumMaxTrials(int trials);                            // sets the maximum number of trials, i.e., the length of the running average
        void    SetDTWinMaxTrials( int trials );
        void    SetIntercept(int controlsigno, float intercept);        // sets the current intercept for this controlsignal
        void    SetGain(int controlsigno, float gain);                  // sets the current gain for this controlsignal
        void    SetWeightControl( FILE * );
        int     GetNumBLstates(int controlsigno);
        int     GetNumTrendstates();
};

#endif

