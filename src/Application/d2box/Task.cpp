/*************************************************************************
Task.cpp for the d2box task
*************************************************************************/
#include <vcl.h>
#pragma hdrstop

#include "Task.h"
#include "Usr.h"
#include "BCIDirectry.h"
#include "UState.h"
#include "MeasurementUnits.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dos.h>
#include <math.h>

//#define  DATAGLOVE

RegisterFilter( TTask, 3 );


TTask::TTask()
: run( 0 ),
  appl( NULL ),
  mVis( SOURCEID::TASKLOG ),
  mpUser( new TUser( Parameters, States ) ),
#ifdef DATAGLOVE
  my_glove( new DataGlove ),
#endif
  OldRunning( 0 ),
  OldCurrentTarget( 0 ),
  oldNtargets( 0 )
{
  BEGIN_PARAMETER_DEFINITIONS
    "UsrTask int PreTrialPause= 10 0 0 0 // "
      "Duration of Target w/o cursor",
    "UsrTask int ItiDuration= 10 0 0 0 // "
      "Duration of Intertrial Interval",
    "UsrTask int RewardDuration= 10 0 0 0 // "
      "Duration of PostTrial Feedback",

    "UsrTask int FeedbackDuration= 20000 0 0 0 // "
      "Max Trial Duration",

    "UsrTask int BaselineInterval= 1 0 0 2 // "
      "Intercept Computation 1 = targets 2 = ITI (enumeration)",
    "UsrTask int TimeLimit= 180 180 0 1000 // "
      "Time Limit for Runs in seconds",
    "UsrTask int RestingPeriod= 0 0 0 1 // "
      "1 defines a rest period of data acquisition (boolean)",

  #ifdef DATAGLOVE
    "JoyStick string GloveCOMport= COM2 0 % % // "
      "COM port for 5DT glove",
    "JoyStick int UseJoyStick= 0 0 0 2 // "
      "0=brain signals; 1=Joystick; 2=Glove (enumeration)",
  #else // DATAGLOVE
    "JoyStick int UseJoyStick= 0 0 0 1 // "
      "0=brain signals; 1=Joystick (enumeration)",
  #endif // DATAGLOVE
    "JoyStick float JoyXgain= 1.0 0 -1000.0 1000.0 // "
      "Horizontal gain",
    "JoyStick float JoyYgain= 1.0 0 -1000.0 1000.0 // "
      "Vertical gain",
    "JoyStick float XOffset= 0 0 -1000.0 1000.0 // "
      "horizontal offset for joystick/glove",
    "JoyStick float YOffset= 0 0 -1000.0 1000.0 // "
      "horizontal offset for joystick/glove",
     "Targets int NumberTargets= 4 0 0 16 // "
      "Number of Targets",
    "Targets int ShowAllTargets= 0 0 0 1 // "
      "Display all Targets? (boolean)",
    "Targets int IncludeAllTargets= 0 0 0 1 // "
      "Test all target positions? (boolean)",
    "Targets int FeedbackMode= 0 0 0 1 // "
      "0 correct hits only 1 any hit (enumeration)",
    "Targets float StartCursorX= 50.0 0 0 100.0 // "
      "Horizontal Start of Cursor",
    "Targets float StartCursorY= 50.0 0 0 100.0 // "
      "Vertical Cursor Starting Position",
    "Targets matrix TargetPos= "
      "{ H%20Position V%20Position H%20Size V%20Size X%20Adapt Y%20Adapt Adapt%20Code } 4 "
      "25 25  0 90 "
      " 0 90 25 25 "
      "50 50 10 10 "
      "10 10 50 50 "
      " 0  0 -1  1 "
      "-1  1  0  0 "
      " 0  0  0  0 "
      " 0 0 0 // Target Position Matrix - Values are 0-100",

  #ifdef DATAGLOVE
    "JoyStick matrix GloveControlX= "
      "{t-1 t sign} " // row labels
      "{ thumb index middle ring little pitch roll } " // column labels
      " -1 0 0 0 0 0 0 "
      "  1 0 0 0 0 0 0 "
      "  1 1 1 1 1 1 1 "
      " 0 0 0 // glove sensor weights for horizontal movement",

    "JoyStick matrix GloveControlY= "
      "{t-1 t sign} " // row labels
      "{ thumb index middle ring little pitch roll } " // column labels
      " 0 0 -1 0 0 0 0 "
      " 0 0  1 0 0 0 0 "
      " 1 1  1 1 1 1 1 "
      " 0 0 0 // glove sensor weights for vertical movement",
  #endif // DATAGLOVE
  END_PARAMETER_DEFINITIONS

  BEGIN_STATE_DEFINITIONS
  #ifdef DATAGLOVE
    "GloveSensor1 8 0 0 0",
    "GloveSensor2 8 0 0 0",
    "GloveSensor3 8 0 0 0",
    "GloveSensor4 8 0 0 0",
    "GloveSensor5 8 0 0 0",
    "GloveSensor6 8 0 0 0",
    "GloveSensor7 8 0 0 0",
  #endif // DATAGLOVE

    "TargetCode 5 0 0 0",
    "ResultCode 5 0 0 0",
    "ResponseTime 8 0 0 0",
    "StimulusTime 16 17528 0 0",
    "Feedback 2 0 0 0",
    "IntertrialInterval 2 1 0 0",
    "RestPeriod 2 0 0 0",
    "CursorPosX 16 0 0 0",
    "CursorPosY 16 0 0 0",

    "Xadapt 16 0 0 0",
    "Yadapt 16 0 0 0",
    "AdaptCode 5 0 0 0",
  END_STATE_DEFINITIONS
}

//-----------------------------------------------------------------------------

TTask::~TTask( void )
{
#ifdef DATAGLOVE
  delete my_glove;
#endif
  delete mpUser;
}

void TTask::Preflight( const SignalProperties& inputProperties,
                             SignalProperties& outputProperties ) const
{
  // External parameters.
  Parameter( "FileInitials" );
  Parameter( "SubjectSession" );
  Parameter( "SubjectName" );

  // External states.
  State( "IntertrialInterval" );
  State( "Running" );

  // TTask::Process() implies that the input signal has at least two integer channels
  // with one element each.
  PreflightCondition( inputProperties >= SignalProperties( 2, 1, SignalType::int16 ) );

  #ifdef DATAGLOVE
  // test for presence of glove if we want to use the glove
  if (Parameter( "UseJoystick" ) == 2)
     {
     PreflightCondition( my_glove->GlovePresent(AnsiString((const char *)Parameter("GloveCOMport"))) );
     PreflightCondition( Parameter("GloveControlX")->GetNumRows() == 3);
     PreflightCondition( Parameter("GloveControlX")->GetNumColumns() == MAX_GLOVESENSORS);
     PreflightCondition( Parameter("GloveControlY")->GetNumRows() == 3);
     PreflightCondition( Parameter("GloveControlY")->GetNumColumns() == MAX_GLOVESENSORS);
     }
  #endif

  // We connect the input signal through to the output signal.
  outputProperties = inputProperties;
}


void TTask::Initialize()
{
        targetDisplay= 0;
        AnsiString FInit,SSes,SName,AName;
        AnsiString COMport;
        time_t ctime;
        struct tm *tblock;
        int i,j;

        PtpDuration=       MeasurementUnits::ReadAsTime( Parameter( "PreTrialPause" ) );
        ItiDuration=       MeasurementUnits::ReadAsTime( Parameter( "ItiDuration" ) );
        OutcomeDuration=   MeasurementUnits::ReadAsTime( Parameter( "RewardDuration" ) );
        FeedbackDuration=  MeasurementUnits::ReadAsTime( Parameter( "FeedbackDuration" ) );
        Ntargets=          Parameter( "NumberTargets" );
        CursorStartX=      Parameter( "StartCursorX" );
        CursorStartY=      Parameter( "StartCursorY" );
        targetDisplay=     Parameter( "ShowAllTargets" );
        feedbackmode=      Parameter( "FeedbackMode" );
        targetInclude=     Parameter( "IncludeAllTargets" );
        BaselineInterval=  Parameter( "BaselineInterval" );
        Resting=           Parameter( "RestingPeriod" );

        timelimit=         Parameter( "TimeLimit" );

        FInit= ( const char* )Parameter( "FileInitials" );
        SSes = ( const char* )Parameter( "SubjectSession" );
        SName= ( const char* )Parameter( "SubjectName" );

        useJoy= Parameter( "UseJoyStick" );
        #ifdef DATAGLOVE
        if ((useJoy < 0) || (useJoy > 2))
          bcierr << "UseJoyStick has to be 0, 1, or 2" << std::endl;
        #else
        if ((useJoy < 0) || (useJoy > 1))
          bcierr << "UseJoyStick has to be 0, or 1" << std::endl;
        #endif

        #ifdef DATAGLOVE
        // transfer the weights for glove into local variables
        if (useJoy == 2)
           {
           COMport= ( const char* )Parameter( "GloveCOMport" );
           for (int sensor=0; sensor<MAX_GLOVESENSORS; sensor++)
            {
            for (int i=0; i<3; i++)
             {
             GloveControlX[i][sensor]=Parameter("GloveControlX", i, sensor);
             GloveControlY[i][sensor]=Parameter("GloveControlY", i, sensor);
             }
            AnsiString glovestatename="GloveSensor"+AnsiString(sensor+1);
            State(glovestatename.c_str())=0;
            }
           }
        #endif

        joy_xgain= Parameter( "JoyXgain" );
        joy_ygain= Parameter( "JoyYgain" );
        XOffset= Parameter( "XOffset" );
        YOffset= Parameter( "YOffset" );

        n_tmat= Ntargets;
        m_tmat= 7;              // was 4;   - now includes width and height  and adaptation code

        for( i= 0; i<m_tmat; i++)
        {
                for(j=0; j<n_tmat; j++)
                {
                        tmat[i][j]= Parameter("TargetPos",i,j);
                }
        }

        if( appl == NULL )
        {
                char FName[120];
                BCIDtry bcidtry;

                bcidtry.SetDir( FInit.c_str() );
                bcidtry.ProcPath();
                bcidtry.SetName( SName.c_str() );
                bcidtry.SetSession( SSes.c_str() );

                strcpy(FName, bcidtry.ProcSubDir() );

                strcat(FName,"\\");

                AName= SName + "S" + SSes + ".apl";
                strcat(FName, AName.c_str() );               // cpy vs cat

                if( (appl= fopen(FName,"a+")) != NULL)       // don't crash if NULL
                {
                  fprintf(appl,"%s \n",AName.c_str() );

                  ctime= time(NULL);
                  tblock= localtime(&ctime);
                  fprintf(appl,"%s \n", asctime( tblock ) );
                }
                else
                  bcierr << "Could not open " << FName << " for writing" << std::endl;
        }

        bitrate.Initialize(Ntargets);

        trial=1;

        mVis.Send( CFGID::WINDOWTITLE, "User Task Log" );

        mpUser->Initialize( Parameters, States );
        mpUser->GetLimits( &limit_right, &limit_left, &limit_top, &limit_bottom );
        mpUser->GetSize( &size_right, &size_left, &size_top, &size_bottom );
        mpUser->Scale( (float)0x7fff, (float)0x7fff );
        ComputeTargets( Ntargets );

        if( targetDisplay == 1 )
        {
                for(i=0;i<Ntargets;i++)
                {
                      mpUser->makeFoil( i+1,
                                targx[i+1],
                                targy[i+1],
                                targsizex[i+1],
                                targsizey[i+1]  );
                }

        }


        if( oldNtargets != Ntargets )
        {
                targetcount= 0;
                ranflag= 0;
        }

        cursor_x_start= ( limit_right - limit_left ) * CursorStartX/100.0;
        cursor_y_start= ( limit_bottom - limit_top ) * CursorStartY/100.0;

        time( &ctime );
        randseed= -ctime;

        ReadStateValues();

        CurrentTarget= 0;
        TargetTime= 0;
        CurrentBaseline= 0;
        BaselineTime= 0;
        CurrentFeedback= 0;
        FeedbackTime= 0;
        CurrentIti= 1;
        ItiTime= 0;
        CurrentFeedback= 0;
        CurrentOutcome= 0;
        OutcomeTime= 0;
        CurrentRest= Resting;
        CurRunFlag= 0;
        Hits= 0;
        Misses= 0;

        PtpTime= 0;

        ntrials= 0;
        ntimeouts= 0;
        totalfeedbacktime= 0;
        feedflag= 0;

        CurrentXadapt= 0;
        CurrentYadapt= 0;
        CurrentAdaptCode= 0;

        mpUser->PutO(false);

        #ifdef DATAGLOVE
        // if we want glove input, start streaming values from the joystick
        if (useJoy == 2)
           {
           delete my_glove;
           my_glove=new DataGlove();            // have to create it new; once stopped streaming, thread is terminated
           my_glove->StartStreaming(COMport);
           }
        #endif

        WriteStateValues();
}

void TTask::ReadStateValues()
{
        CurrentTarget=       State("TargetCode");
        CurrentOutcome=      State("ResultCode");
        CurrentStimulusTime= State("StimulusTime");
        CurrentFeedback=     State("Feedback");
        CurrentIti=          State("IntertrialInterval");
        CurrentRunning=      State("Running");
                if( CurRunFlag == 1 )     // 0 must cycle through at least once
                {
                        if( CurrentRunning == 1 ) CurrentRunning = 0;
                        else                      CurRunFlag= 0;
                }

        CurrentRest=         State("RestPeriod");

        CurrentXadapt=       State("Xadapt");
        CurrentYadapt=       State("Yadapt");
        CurrentAdaptCode=    State("AdaptCode");

}

void TTask::WriteStateValues()
{
        CurrentStimulusTime= BCITIME::GetBCItime_ms();                   // time stamp

        State("StimulusTime")=CurrentStimulusTime;

        State("TargetCode")=CurrentTarget;
        State("ResultCode")=CurrentOutcome;
        State("ResponseTime")=FeedbackTime;
        State("Feedback")=CurrentFeedback;
        State("IntertrialInterval")=CurrentIti;
        State("Running")=CurrentRunning;
        State("RestPeriod")=CurrentRest;
        State("CursorPosX")=x_pos;
        State("CursorPosY")=y_pos;

        State("Xadapt")=CurrentXadapt;
        State("Yadapt")=CurrentYadapt;
        State("AdaptCode")=CurrentAdaptCode;

}

void TTask::ComputeTargets( int ntargs )
{
        int i;
        float y_range;
        float x_range;

        y_range= size_bottom - size_top;
      //  y_range= y_range- (1.0 * TargetWidth);

        x_range= size_right - size_left;
     //   x_range= x_range - (1.0 * TargetHeight);

        for(i=0;i<ntargs;i++)
        {
                targx[i+1]=          tmat[0][i] * x_range / 100;
                targsizex[i+1]=      tmat[2][i] * x_range / 100;
                targx_rt[i+1]=       targx[i+1] + targsizex[i+1];
                targy[i+1]=          tmat[1][i] * y_range / 100;
                targsizey[i+1]=      tmat[3][i] * y_range / 100;
                targy_btm[i+1]=      targy[i+1] + targsizey[i+1];
                targx_adapt[i+1]=    tmat[4][i];
                targy_adapt[i+1]=    tmat[5][i];
                targ_adaptcode[i+1]= tmat[6][i];
        }

}

void TTask::ShuffleTargs( int ntargs )
{
        int i,j;
        float rval;

        for(i=0;i<ntargs;i++)
        {
                rpt:    rval= mpUser->ran1( &randseed );
                        targs[i]= 1 + (int)( rval * ntargs );

                if( (targs[i] < 1) || (targs[i]>Ntargets) )
                        goto rpt;

                for(j=0;j<i; j++)
                         if( targs[j]==targs[i] ) goto rpt;
        }
}

int TTask::GetTargetNo( int ntargs )
{
        int retval;

        if( ranflag == 0 )
        {
                ShuffleTargs( ntargs );
                ranflag= 1;
        }

        retval= targs[targetcount];

        targetcount++;
        if( targetcount > (ntargs-1) )
        {
                ranflag= 0;
                targetcount= 0;
        }

        return( retval );

}

int TTask::TestTarget( float xpos, float ypos, int targ )
{
        int result;
        int half;

        half= mpUser->HalfCursorSize;

        result= 0;

        // could use cursor center by adding 1/2 of size to position

        if( ( (xpos+half) > targx[targ] ) && ( (xpos-half) < targx_rt[targ] ) )
        {
                if( ( (ypos+half) > targy[targ] ) && ( (ypos-half) < targy_btm[targ] ) ) result= 1;
        }

        return( result );
}


void TTask::TestCursorLocation( float x, float y )
{
        int i;
        int res;

 //       x*= User->scalex;
 //       y*= User->scaley;


        if( targetInclude == 0 )
        {
                res= TestTarget( x, y, CurrentTarget );
                if( res == 1 )
                {
                        mpUser->Clear();
                        mpUser->PutCursor( &x_pos, &y_pos, clBlue );
                        mpUser->PutTarget( targx[ CurrentTarget ],
                                 targy[ CurrentTarget ],
                                 targsizex[ CurrentTarget ],
                                 targsizey[ CurrentTarget ],
                                 clYellow   );
                        CurrentFeedback= 0;
                        CurrentOutcome= CurrentTarget;
                        if( BaselineInterval == 1 )
                                CurrentBaseline= 0;
                }
        }


        else if( targetInclude == 1 )
        {

                res= TestTarget( x, y, CurrentTarget );         // test correct first
                if ( res == 1 )
                {
                        CurrentOutcome= CurrentTarget;
                        goto jmp;
                }

                for(i=1; i<Ntargets+1; i++)
                {
                        if( TestTarget( x, y, i ) == 1 )
                        {
                                CurrentOutcome= i;
                                goto jmp;
                        }
                }

                return;

jmp:            if( CurrentOutcome == CurrentTarget )
                {
                         mpUser->Clear();
                         mpUser->PutTarget( targx[ CurrentTarget ],
                                 targy[ CurrentTarget ],
                                 targsizex[ CurrentTarget ],
                                 targsizey[ CurrentTarget ],
                                 clYellow   );
                          Hits++;
                          HitOrMiss=true;
                }
                else
                {
                         mpUser->Clear();

                         if( feedbackmode == 0 )
                         {
                                mpUser->PutTarget( targx[ CurrentTarget ],
                                        targy[ CurrentTarget ],
                                        targsizex[ CurrentTarget ],
                                        targsizey[ CurrentTarget ],
                                        clBlack   );
                                Misses++;
                                HitOrMiss=false;
                         }
                         else
                         {
                                mpUser->PutTarget( targx[ CurrentOutcome ],
                                        targy[ CurrentOutcome ],
                                        targsizex[ CurrentOutcome ],
                                        targsizey[ CurrentOutcome ],
                                        clYellow   );
                                Misses++;
                                HitOrMiss=false;
                         }
                }
                CurrentFeedback= 0;
                if( BaselineInterval == 1 )
                        CurrentBaseline= 0;
        }

}

void TTask::UpdateSummary( void )
{
        float pc;

        fprintf(appl," @ D2Box @   Number of Targets=%2d \n",Ntargets);

        if( targetInclude == 0 )
        {
                fprintf(appl,"NoError Mode Run %2d  Trials  %3d  Timeouts  %3d \n",run,ntrials,ntimeouts);
                if( ntrials > 0 ) fprintf(appl,"   Mean trial duration= %6.2f \n",totalfeedbacktime/(double)(ntrials) );
        }
        else
        {

                if( Hits+Misses > 0 )    pc= 100.0 * Hits / (Hits+Misses);
                else                     pc= -1.0;

                fprintf(appl,"Run %2d   Hits=%3d  Total=%3d  Percent=%6.2f \n",run,Hits,Hits+Misses,pc);
                fprintf(appl,"Bits= %7.2f \n",bitrate.TotalBitsTransferred() );
        }



        fprintf(appl,"Time Passed (sec)=%7.2f \n",timepassed);
        fprintf(appl,"..........................................................\n\n");
}

void TTask::UpdateDisplays( void )
{

char            memotext[256];


 if (CurrentRunning == 0)
    CurrentRunning=0;

 if ((CurrentRunning == 0) && (OldRunning == 1))            // put the T up there on the transition from not suspended to suspended
    {
    mpUser->Clear();
    mpUser->PutT(true);
    CurrentIti=1;
    }
 if (CurrentRunning == 1)
    {
    if (OldRunning == 0)
       {
       runstarttime=(double)(TDateTime::CurrentTime());   // run just started ?
       run++;
       }
    mpUser->PutT(false);
    // in the ITI period ?
    if (CurrentIti > 0)
       {
       // ITI period just started
       if (ItiTime == 0)
          {
          sprintf(memotext, "Run: %d; ITI -> new trial: %d\r", run, trial);
          mVis.Send(memotext);
          trial++;
          }
            // in case the run was longer than x seconds
            timepassed= ((double)(TDateTime::CurrentTime())-runstarttime)*86400;

            if (timepassed > timelimit)
               {
               CurrentRunning=0;
               CurRunFlag= 1;
               if (Hits+Misses > 0)
                  {
                  sprintf(memotext, "Run %d - %.1f%% correct\r", run, (float)Hits*100/((float)Hits+(float)Misses));
                  mVis.Send(memotext);
              //    fprintf( appl,"%s \n",memotext);
                  }
               else
                  {
                  mVis.Send("No statistics for this run\r");
                  mVis.Send("There were no trials\r");
                  }
               sprintf(memotext, "Total Bits transferred: %.2f\r", bitrate.TotalBitsTransferred());
               mVis.Send(memotext);
            //   fprintf( appl,"%s \n",memotext);
               sprintf(memotext, "Average Bits/Trial: %.2f\r", bitrate.BitsPerTrial());
               mVis.Send(memotext);
           //    fprintf( appl,"%s \n",memotext);
               sprintf(memotext, "Average Bits/Minute: %.2f\r", bitrate.BitsPerMinute());
               mVis.Send(memotext);
            //   fprintf( appl,"%s \n",memotext);
               mVis.Send("**************************\r");
             //  fprintf( appl,"**************************\r\n");
               mpUser->Clear();
               mpUser->PutT(true);
               UpdateSummary();
               }
        }

    else if (CurrentOutcome > 0 )
            {
            // trial just ended ...
            if (OutcomeTime == 0)
               {
               sprintf(memotext, "%d hits %d missed\r", Hits, Misses);
               mVis.Send(memotext);
               bitrate.Push(HitOrMiss);
               }

            }
    }

 // new trial just started
 if ((CurrentTarget > 0) && (OldCurrentTarget == 0))
    {
    sprintf(memotext, "Target: %d\r", CurrentTarget);
    mVis.Send(memotext);
    }

 OldRunning=CurrentRunning;
 OldCurrentTarget=CurrentTarget;
}

void TTask::Iti( void )
{
        int i;
        TColor clr;

        if( BaselineInterval == 2 )
                CurrentBaseline= 1;
        mpUser->Clear();
        ItiTime++;

        if( ItiTime > ItiDuration )
        {
                FeedbackTime= 0;
                CurrentIti = 0;
                ItiTime = 0;
                CurrentTarget= GetTargetNo( Ntargets );

                if( targetDisplay == 1 )
                {
                        for(i=0;i<Ntargets;i++)
                        {
                                if( CurrentTarget == (i+1) )
                                        mpUser->PutFoils( i+1, clRed   );
                                else
                                        mpUser->PutFoils( i+1, clGreen   );
                        }

                }

                else
                {
                        mpUser->PutTarget( targx[ CurrentTarget ],
                                 targy[ CurrentTarget ],
                                 targsizex[ CurrentTarget ],
                                 targsizey[ CurrentTarget ],
                                 clRed   );
                }

                if( BaselineInterval == 1 )
                        CurrentBaseline= 1;
                if( BaselineInterval == 2 )
                        CurrentBaseline= 0;


                CurrentXadapt= targx_adapt[CurrentTarget];
                CurrentYadapt= targy_adapt[CurrentTarget];
                CurrentAdaptCode= targ_adaptcode[CurrentTarget];
         }
         if( (targetInclude == 0) && (feedflag == 1) )
         {
                totalfeedbacktime+= ((double)(TDateTime::CurrentTime() ) - feedstart ) * 86400;
                feedflag= 0;
         }
}

void TTask::Ptp( void )
{
        MMRESULT mmres;
        JOYINFO joyi;

        PtpTime++;

        if( PtpTime > PtpDuration )
        {

                x_pos= cursor_x_start;
                y_pos= cursor_y_start;
                mpUser->PutCursor( &x_pos, &y_pos, clRed );
                CurrentFeedback= 1;
                FeedbackTime= 0;

                PtpTime= 0;

                if( useJoy == 1 )   // try && (PtpTime == 1) )
                {
                         // center the joystick based on the current joystick position
                         // mmres= joyGetPos( JOYSTICKID1, &joyi );
                         // jx_cntr= joyi.wXpos-32768;
                         // jy_cntr= joyi.wYpos-32768;
                         // center the joystick based on user-defined parameters;
                         jx_cntr=(int)XOffset;
                         jy_cntr=(int)YOffset;
                }
        }

}


void TTask::GetJoy( )
{
        JOYINFO joyi;
        float tx;
        float ty;

        joyGetPos( JOYSTICKID1, &joyi );

        tx= (float)(joyi.wXpos)-32768  - (float)(jx_cntr);
        ty= (float)(joyi.wYpos)-32768  - (float)(jy_cntr);

        tx= tx / (float)jhalf;
        ty= ty / (float)jhalf;

        x_pos+= tx * joy_xgain;
        y_pos+= ty * joy_ygain;
}


#ifdef DATAGLOVE
void TTask::GetGlove( )
{
float cur_glovevalx, cur_glovevaly;

        cur_glovevalx=0;
        cur_glovevaly=0;
        // we are doing the following transformation:
        // deltax=sum(abs(sensor(t)*weight1(t)+sensor(t-1)*weight2(t-1))*sign) with sign determined by the parameter's third row
        for (int i=0; i<MAX_GLOVESENSORS; i++)
            {
            float old_sensorval=(float)my_glove->GetSensorValOld(i);
            float cur_sensorval=(float)my_glove->GetSensorVal(i);
            float cur_signx, cur_signy;
            if (GloveControlX[2][i] >= 0) cur_signx=1;
            else cur_signx=-1;
            if (GloveControlY[2][i] >= 0) cur_signy=1;
            else cur_signy=-1;
            cur_glovevalx += fabs(GloveControlX[0][i]*old_sensorval+GloveControlX[1][i]*cur_sensorval)*cur_signx;
            cur_glovevaly += fabs(GloveControlY[0][i]*old_sensorval+GloveControlY[1][i]*cur_sensorval)*cur_signy;
            AnsiString glovestatename="GloveSensor"+AnsiString(i+1);
            State(glovestatename.c_str())=(unsigned short)my_glove->GetSensorVal(i);
            }
        // my_glove->ReadSensorsFromGlove("COM2");
        x_pos+= (cur_glovevalx-XOffset) * joy_xgain;
        y_pos+= (cur_glovevaly-YOffset) * joy_ygain;
        if (my_glove->GetGloveErr() != GLOVE_ERR_NOERR)
           Application->MessageBox("Glove Error", MB_OK);
}
#endif

void TTask::Feedback( short sig_x, short sig_y )
{
        if(useJoy == 0)
        {
                x_pos+= (float)sig_x * mpUser->scalex;
                y_pos+= (float)sig_y * mpUser->scaley;
        }
        if(useJoy == 1) GetJoy();
        #ifdef DATAGLOVE
        if(useJoy == 2) GetGlove();
        #endif

        mpUser->PutCursor( &x_pos, &y_pos, clRed );
        TestCursorLocation( x_pos, y_pos );

        FeedbackTime++;

        if( FeedbackTime > FeedbackDuration )
        {
                //   FeedbackTime= 0;
                CurrentFeedback= 0;
                CurrentOutcome= 0;
                CurrentIti= 1;
                ntimeouts++;
        }
        if( feedflag == 0 )
        {
                feedflag= 1;
                ntrials++;
                feedstart= (double)(TDateTime::CurrentTime());
        }
}


void TTask::Outcome()
{
        OutcomeTime++;

        if( OutcomeTime > OutcomeDuration )
        {
                CurrentIti= 1;
                CurrentOutcome= 0;
                CurrentTarget= 0;
                OutcomeTime= 0;
                mpUser->PutCursor( &x_pos, &y_pos, clBlack );
        }

        if( (targetInclude == 0) && (feedflag == 1) )
        {
                totalfeedbacktime+= ((double)(TDateTime::CurrentTime() ) - feedstart ) * 86400;
                feedflag= 0;
        }
}

void TTask::Rest( void )
{
        if ((CurrentRunning == 1) && (OldRunning == 0))
        {
                runstarttime=(double)(TDateTime::CurrentTime());   // run just started ?
                run++;
                OldRunning= 1;
                mpUser->Clear();
                mpUser->PutT(false);
                mpUser->PutO(true);
               // CurrentRest= 1;
                CurrentIti= 0;

        }

        timepassed= ((double)(TDateTime::CurrentTime())-runstarttime)*86400;

         if (timepassed > timelimit)
         {
               CurrentRunning=0;
               mpUser->Clear();
               mpUser->PutO(false);
               mpUser->PutT(true);
               CurrentRest= 0;
               CurrentIti= 1;
         }

}

void TTask::Process( const GenericSignal * Input, GenericSignal * Output )
{
        ReadStateValues();

        if( CurrentRunning > 0 )
        {
                if(CurrentRest > 0 )             Rest();
                else if (CurrentIti > 0)         Iti();
                else if (CurrentFeedback > 0 )   Feedback( ( *Input )( 1, 0 ), ( *Input )( 0, 0 ) );
                else if (CurrentOutcome  > 0 )   Outcome();
                else if (CurrentTarget   > 0 )   Ptp();
        }

        UpdateDisplays();
        WriteStateValues();
        *Output = *Input;
}


