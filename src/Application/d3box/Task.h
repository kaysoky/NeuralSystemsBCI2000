/*************************************************************************
Task.h is the header file for the Right Justified Boxes task
*************************************************************************/
#ifndef TaskH
#define TaskH

#include <stdio.h>
#include "UBitRate.h"
#include "UGenericVisualization.h"
#include "UDataGlove.h"
#include "UGenericFilter.h"
#include "UBCItime.h"

#include <mmsystem.h>

#define jhalf 32767
#define NTARGS  64
/*shidong starts*/
#define COLORFORMAT 8
/*shidong ends*/
class TTask : public GenericFilter
{
private:
        int useJoy;
        int targetInclude;
        float joy_xgain;
        float joy_ygain;
        float joy_zgain;
        float x_pos, x_trkpos, x_radius;
        float y_pos, y_trkpos, y_radius;
        float cursor_x_start;
        float cursor_y_start;

        bool  TrackingTarget;
        float ellipse_radius_x;
        float ellipse_radius_y;
        float ellipse_radius_decrement_x;
        float ellipse_radius_decrement_y;
        float tracking_speed;
        int   tracking_shape;
        float triangle_size_x, triangle_size_y;

        float cntr;             // counter for drawing tracking figures

        float CursorStartX;
        float CursorStartY;
        float CursorStartZ;
        float CursorRadius;

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
        short targz_adapt[NTARGS+1];
        short targ_adaptcode[NTARGS+1];

        // weights for glove control
        float GloveControlX[3][MAX_GLOVESENSORS];
        float GloveControlY[3][MAX_GLOVESENSORS];

        // offset for joystick/glove
        float XOffset, YOffset, ZOffset;

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
        int tmat[10][NTARGS];

        unsigned short OldRunning;
        unsigned short OldCurrentTarget;

        bool HitOrMiss;                          // after trial: true, if hit; false if miss

        long randseed;
        /*shidong starts*/
        AnsiString borderTexture;
        AnsiString targetTexture;
        AnsiString cursorTexture;

        int WorkspaceBoundaryVisible;            // 1 means show the 3D workspace, 0 means not show
        AnsiString CursorColorFront;                  // cursor's color when it is at front of the workspace, i.e Z= -32767 or -0x7FFF
        AnsiString CursorColorBack;                   // cursor's color when it is at front of the workspace, i.e Z= 32767 or 0x7FFF
        int WinHeight;
        int WinWidth;
        int WinXpos;
        int WinYpos;
        int WindowFullScreen;
        int DisplayMonitor;
        int ChangeResolution;
        /*shidong ends*/


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
       // int Resting;                          // value of parameter for resting data acquisition period
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
        short CurrentZadapt;
        unsigned CurrentAdaptCode;

        int TestTarget( float, float, int );
        void GetJoy( void );
        void GetGlove( );
        void GetMouse( void );
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
        BITRATE         bitrate;
        BCITIME         *bcitime;
        TApplication    *Applic;
        DataGlove       *my_glove;

        /*shidong starts*/
        int CameraX;
        int CameraY;
        int CameraZ;
        int CameraAimX;
        int CameraAimY;
        int CameraAimZ;
        int LightSourceX;
        int LightSourceY;
        int LightSourceZ;
        int LightSourceColorR;
        int LightSourceColorG;
        int LightSourceColorB;
        int LightSourceIntensity;

public:
        TTask();
        virtual ~TTask();

        void ReadStateValues(STATEVECTOR *);
        void WriteStateValues(STATEVECTOR *);
        virtual void Preflight( const SignalProperties&, SignalProperties& )const;
        virtual void Initialize();
        virtual void Process( const GenericSignal * Input, GenericSignal * Output );

        /*shidong starts*/
        bool checkPath(AnsiString path) ;
        void checkPathHelper();
        bool checkInt(AnsiString input, AnsiString paraName) const;
        /*shidong ends*/
} ;

#endif
