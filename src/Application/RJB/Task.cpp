/*************************************************************************
Task.cpp is the source code for the Right Justified Boxes task
*************************************************************************/
#include "PCHIncludes.h"
#pragma hdrstop

#include "Task.h"
#include "Usr.h"
#include "UState.h"
#include "BCIDirectry.h"
#include "UBCIError.h"
#include "Localization.h"
#include "MeasurementUnits.h"

#include <vector>
#include <iostream>
#include <iomanip>
#include <time.h>
#include <assert>

using namespace std;

RegisterFilter( TTask, 3 );

TTask::TTask()
: run( 0 ),
  mTaskLog( ".apl" ),
  mVis( SOURCEID_TASKLOG ),
  OldRunning( 0 ),
  OldCurrentTarget( 0 )
{
  BEGIN_PARAMETER_DEFINITIONS
    "UsrTask int PreRunInterval= 20 0 0 100 // "
        "Pause prior to starting a run (in units of SampleBlocks)",
    "UsrTask int PreTrialPause= 10 0 0 100 // "
        "Duration of Target w/o cursor",
    "UsrTask int ItiDuration= 10 0 0 100 // "
        "Duration of Intertrial Interval",
    "UsrTask int RewardDuration= 10 0 0 100 // "
        "Duration of PostTrial Feedback",
    "UsrTask int NumberTargets= 2 0 0 1023 // "
        "Number of Targets",
    "UsrTask float TargetWidth= 25 0 0 32767 // "
        "Width of Targets",
    "UsrTask int BaselineInterval= 1 0 0 2 // "
        "Intercept Computation 1 = targets 2 = ITI",
    "UsrTask int TimeLimit= 180 180 0 1000 // "
        "Time Limit for Runs in seconds",
    "UsrTask int RestingPeriod= 0 0 0 1 // "
        "1 defines a rest period of data acquisition",
    "UsrTask matrix Announcements= "
      " { 0 1 2 } { Task Result } "
      "     %      % "
      "     %      % "
      "     %      % "
      "// Wave files for auditory announcements",
  END_PARAMETER_DEFINITIONS

  BEGIN_STATE_DEFINITIONS
    "TargetCode 5 0 0 0",
    "ResultCode 5 0 0 0",
    "StimulusTime 16 17528 0 0",
    "Feedback 2 0 0 0",
    "IntertrialInterval 2 0 0 0",
    "RestPeriod 2 0 0 0",
    "CursorPosX 16 0 0 0",
    "CursorPosY 16 0 0 0",
  END_STATE_DEFINITIONS

  LANGUAGES "French",
            "German",
            "Dutch",
  BEGIN_LOCALIZED_STRINGS
    "YES",
            "OUI",
            "JA",
            "JA",
    "NO",
            "NON",
            "NEIN",
            "NEE",
    "N/A",
            " - ",
            " - ",
            " - ",
    "Get Ready ...",
            "Appr" ecirc "ter ...",
            "Achtung ...",
            "Opgepast ...",
  END_LOCALIZED_STRINGS

  assert( User == NULL );
  Application->CreateForm( __classid( TUser ), &User );
  User->SetUsr( Parameters, States );
}

//-----------------------------------------------------------------------------

TTask::~TTask( void )
{
}

void
TTask::Preflight( const SignalProperties& inputProperties,
                        SignalProperties& outputProperties ) const
{
  TWavePlayer preflightPlayer;
  for( size_t i = 0; i < Parameter( "Announcements" )->GetNumValuesDimension1(); ++i )
    for( size_t j = 0; j < Parameter( "Announcements" )->GetNumValuesDimension2(); ++j )
  {
    string soundFile = static_cast<const char*>( Parameter( "Announcements", i, j ) );
    if( soundFile != "" )
      if( preflightPlayer.AttachFile( soundFile.c_str() ) != TWavePlayer::noError )
        bcierr << "Could not open " << soundFile << " for audio playback" << endl;
  }
  mTaskLog.Preflight();
  outputProperties = SignalProperties( 0, 0 );
}

void TTask::Initialize()
{
  ApplyLocalizations( User );

  PreRunInterval =   MeasurementUnits::ReadAsTime( Parameter( "PreRunInterval" ) );
  PtpDuration =      MeasurementUnits::ReadAsTime( Parameter( "PreTrialPause" ) );
  ItiDuration =      MeasurementUnits::ReadAsTime( Parameter( "ItiDuration" ) );
  OutcomeDuration =  MeasurementUnits::ReadAsTime( Parameter( "RewardDuration" ) );
  Ntargets =         Parameter("NumberTargets");
  TargetWidth =      Parameter( "TargetWidth" );
  BaselineInterval = Parameter( "BaselineInterval" );
  Resting =          Parameter( "RestingPeriod" );

  timelimit =        Parameter( "TimeLimit" );

  mTaskAnnouncements.resize( Ntargets + 1 );
  mResultAnnouncements.resize( Ntargets + 1 );
  for( int i = 0; i <= Ntargets; ++i )
  {
    mTaskAnnouncements[ i ].AttachFile( Parameter( "Announcements", i, "Task" ) );
    mResultAnnouncements[ i ].AttachFile( Parameter( "Announcements", i, "Result" ) );
  }

  mTaskLog.Initialize();
  bitrate.Initialize(Ntargets);

  trial=1;

  mVis.Send( CFGID::WINDOWTITLE, "User Task Log" );

  User->Initialize( Parameters, States );
  User->GetLimits( &limit_right, &limit_left, &limit_top, &limit_bottom );
  User->Scale( (float)0x7fff, (float)0x7fff );
  ComputeTargets( Ntargets );
  targetcount= 0;
  ranflag= 0;

  time_t ctime= time(NULL);
  time( &ctime );
  randseed= -ctime;

  cursor_x_start= limit_left;
  cursor_y_start= ( limit_top + limit_bottom ) /2;

  ReadStateValues( Statevector );

  CurrentTarget= 0;
  TargetTime= 0;
  CurrentBaseline= 0;
  BaselineTime= 0;
  CurrentFeedback= 0;
  FeedbackTime= 0;
  // if there is no PreRunInterval (i.e., it is 0), switch to ITI directly
  if (PreRunInterval > 0)
     {
     CurrentPri= 1;
     CurrentIti= 0;
     }
  else
     {
     CurrentPri= 0;
     CurrentIti= 1;
     }
  PriTime= 0;
  ItiTime= 0;
  CurrentFeedback= 0;
  FeedbackTime= 0;
  CurrentOutcome= 0;
  OutcomeTime= 0;
  CurrentRest= Resting;
  CurRunFlag= 0;
  Hits= 0;
  Misses= 0;
  User->PutO(false);

  WriteStateValues( Statevector );
}

void TTask::ReadStateValues(STATEVECTOR *statevector)
{
        CurrentTarget=       statevector->GetStateValue("TargetCode");
        CurrentOutcome=      statevector->GetStateValue("ResultCode");
        CurrentStimulusTime= statevector->GetStateValue("StimulusTime");
        CurrentFeedback=     statevector->GetStateValue("Feedback");
        CurrentIti=          statevector->GetStateValue("IntertrialInterval");
        CurrentRunning=      statevector->GetStateValue("Running");
                if( CurRunFlag == 1 )     // 0 must cycle through at least once
                {
                        if( CurrentRunning == 1 ) CurrentRunning = 0;
                        else                      CurRunFlag= 0;
                }

        CurrentRest=         statevector->GetStateValue("RestPeriod");

}

void TTask::WriteStateValues(STATEVECTOR *statevector)
{
        CurrentStimulusTime= BCITIME::GetBCItime_ms();   // time stamp
        statevector->SetStateValue("StimulusTime",CurrentStimulusTime);

        statevector->SetStateValue("TargetCode",CurrentTarget);
        statevector->SetStateValue("ResultCode",CurrentOutcome);
        statevector->SetStateValue("Feedback",CurrentFeedback);
        statevector->SetStateValue("IntertrialInterval",CurrentIti);
        statevector->SetStateValue("Running",CurrentRunning);
        statevector->SetStateValue("RestPeriod",CurrentRest);
        statevector->SetStateValue("CursorPosX",(int)x_pos*User->scale_x );
        statevector->SetStateValue("CursorPosY",(int)y_pos*User->scale_y );
}

void TTask::ComputeTargets( int ntargs )
{
        int i;
        float y_range;

        y_range= limit_bottom - limit_top;

        for(i=0;i<ntargs;i++)
        {
                User->targx[i+1]=      limit_right - ( 1.0 * TargetWidth );
                User->targsizex[i+1]=  TargetWidth;
                User->targy[i+1]=      limit_top + i * y_range/ntargs;
                User->targsizey[i+1]=  y_range/ntargs;
                User->targy_btm[i+1]=  User->targy[i+1]+User->targsizey[i+1];
        }

}

void TTask::ShuffleTargs( int ntargs )
{
        int i,j;
        float rval;

        for(i=0;i<ntargs;i++)
        {
                rpt:    rval= User->ran1( &randseed );
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


void TTask::TestCursorLocation( float x, float y )
{
        int i;

        x*= User->scale_x;
        y*= User->scale_y;

        if( x > ( limit_right - TargetWidth ) )
        {
                User->PutCursor( x_pos, y_pos, CURSOR_RESULT );

                CurrentOutcome= 1;

                if( y < User->targy[1] )              y= User->targy[1];
                if( y > User->targy_btm[Ntargets] )   y= User->targy_btm[Ntargets];

                for(i=0;i<Ntargets;i++)
                {
                        if( ( y > User->targy[i+1] ) && ( y<= User->targy_btm[i+1] ) )
                        {
                                CurrentOutcome= i+1;

                        }
                }

                // flash outcome (only) when YES/NO (display the same for hits and misses)
                User->Outcome(0, CurrentOutcome );

                if( CurrentOutcome == CurrentTarget )
                {
                          User->PutTarget(CurrentTarget, TARGET_RESULT );
                          Hits++;
                          HitOrMiss=true;
                }
                else
                {
                         User->PutTarget(CurrentTarget, TARGET_OFF );
                         Misses++;
                         HitOrMiss=false;
                }
                CurrentFeedback= 0;
                if( BaselineInterval == 1 )
                        CurrentBaseline= 0;
        }

}

void TTask::UpdateSummary( void )
{
        float pc;

        if( Hits+Misses > 0 )    pc= 100.0 * Hits / (Hits+Misses);
        else                     pc= -1.0;

        mTaskLog << "Run " << run
                 << "  Hits=" << Hits
                 << "  Total=" << Hits + Misses
                 << "  Percent=" << setprecision( 2 ) << pc << " \n"
                 << "Number of Targets=" << Ntargets << " \n"
                 << "Bits= " << bitrate.TotalBitsTransferred() << " \n"
                 << "Time Passed (sec)=" << timepassed << " \n"
                 << "..........................................................\n"
                 << endl;
}

void TTask::UpdateDisplays( void )
{
char            memotext[256];

 if (CurrentRunning == 0)
    CurrentRunning=0;

 if ((CurrentRunning == 0) && (OldRunning == 1))            // put the T up there on the transition from not suspended to suspended
    {
    User->Clear();
    User->PutT(true);
    CurrentIti=1;
    CurrentFeedback=0;
    CurrentTarget=0;
    CurrentOutcome=0;
    }
 if (CurrentRunning == 1)
    {
    if (OldRunning == 0)
       {
       runstarttime=(double)(TDateTime::CurrentTime());   // run just started ?
       run++;
       }
    User->PutT(false);
    // in the ITI period ?
    if (CurrentIti > 0)
       {
       // ITI period just started
       if (ItiTime == 0)
          {
          sprintf(memotext, "Run: %d; ITI -> new trial: %d", run, trial);
          mVis.Send( memotext );
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
                  sprintf(memotext, "Run %d - %.1f%% correct", run, (float)Hits*100/((float)Hits+(float)Misses));
                  mVis.Send( memotext );
              //    fprintf( appl,"%s \n",memotext);
                  }
               else
                  {
                  mVis.Send( "No statistics for this run" );
                  mVis.Send( "There were no trials" );
                  }
               sprintf(memotext, "Total Bits transferred: %.2f", bitrate.TotalBitsTransferred());
               mVis.Send( memotext );
            //   fprintf( appl,"%s \n",memotext);
               sprintf(memotext, "Average Bits/Trial: %.2f", bitrate.BitsPerTrial());
               mVis.Send( memotext );
           //    fprintf( appl,"%s \n",memotext);
               sprintf(memotext, "Average Bits/Minute: %.2f", bitrate.BitsPerMinute());
               mVis.Send( memotext );
            //   fprintf( appl,"%s \n",memotext);
               mVis.Send( "**************************" );
             //  fprintf( appl,"**************************\r\n");
               User->Clear();
               User->PutT(true);
               UpdateSummary();
               }
        }

    else if (CurrentOutcome > 0 )
            {
            // trial just ended ...
            if (OutcomeTime == 0)
               {
               mResultAnnouncements[ CurrentOutcome ].Play();
               sprintf(memotext, "%d hits %d missed", Hits, Misses);
               mVis.Send( memotext );
               bitrate.Push(HitOrMiss);
               }

            }
    }

 // new trial just started
 if ((CurrentTarget > 0) && (OldCurrentTarget == 0))
    {
    sprintf(memotext, "Target: %d", CurrentTarget);
    mVis.Send( memotext );
    }

 OldRunning=CurrentRunning;
 OldCurrentTarget=CurrentTarget;
}


// if we have defined this period, show some initial feedback
// after a run has been started
void TTask::Pri( void )
{
 User->PreRunInterval(PriTime);
 PriTime++;

 // set all state variables to 0 in this period
 CurrentIti=0;
 CurrentTarget=0;
 CurrentBaseline=0;
 CurrentFeedback=0;
 CurrentOutcome=0;

 if (PriTime >= PreRunInterval)
    {
    CurrentPri=0;
    CurrentIti=1;
    }
}


void TTask::Iti( void )
{
        if( BaselineInterval == 2 )
                CurrentBaseline= 1;
        User->Clear();
        ItiTime++;

        if( ItiTime > ItiDuration )
        {
                CurrentIti = 0;
                ItiTime = 0;
                CurrentTarget= GetTargetNo( Ntargets );
                User->PutTarget(CurrentTarget, TARGET_ON );
                mTaskAnnouncements[ CurrentTarget ].Play();

                if( BaselineInterval == 1 )
                        CurrentBaseline= 1;
                if( BaselineInterval == 2 )
                        CurrentBaseline= 0;


        }
}

void TTask::Ptp( void )
{
        PtpTime++;

        if( PtpTime > PtpDuration )
        {

                x_pos= cursor_x_start;
                y_pos= cursor_y_start;
                if( User->scale_x > 0.0 ) x_pos/= User->scale_x;
                if( User->scale_y > 0.0 ) y_pos/= User->scale_y;
                User->PutCursor( x_pos, y_pos, CURSOR_ON );
                CurrentFeedback= 1;
                PtpTime= 0;
        }

}

void TTask::Feedback( short sig_x, short sig_y )
{

        x_pos+= sig_x;
        y_pos+= sig_y;
        User->PutCursor( x_pos, y_pos, CURSOR_ON );
        TestCursorLocation( x_pos, y_pos );
}


void TTask::Outcome()
{
        OutcomeTime++;
        User->Outcome( OutcomeTime, CurrentOutcome );                   // flash outcome when YES/NO

        if( OutcomeTime >= OutcomeDuration-1 )
        {
                CurrentIti= 1;
                CurrentOutcome= 0;
                CurrentTarget= 0;
                OutcomeTime= 0;
                User->PutCursor( x_pos, y_pos, CURSOR_OFF );
        }
}

void TTask::Rest( void )
{
        if ((CurrentRunning == 1) && (OldRunning == 0))
        {
                runstarttime=(double)(TDateTime::CurrentTime());   // run just started ?
                run++;
                OldRunning= 1;
                User->Clear();
                User->PutT(false);
                User->PutO(true);
               // CurrentRest= 1;
                CurrentIti= 0;

        }

        timepassed= ((double)(TDateTime::CurrentTime())-runstarttime)*86400;

         if (timepassed > timelimit)
         {
               CurrentRunning=0;
               User->Clear();
               User->PutO(false);
               User->PutT(true);
               CurrentRest= 0;
               CurrentIti= 1;
         }

}

void TTask::Process( const GenericSignal* Input, GenericSignal* Output )
{
        ReadStateValues( Statevector );

        if( CurrentRunning > 0 )
        {
                if(CurrentRest > 0 )             Rest();
                else if (CurrentPri > 0)         Pri();
                else if (CurrentIti > 0)         Iti();
                else if (CurrentFeedback > 0 )   Feedback( ( *Input )( 1, 0 ), ( *Input )( 0, 0 ) );
                else if (CurrentOutcome  > 0 )   Outcome();
                else if (CurrentTarget   > 0 )   Ptp();
        }

        UpdateDisplays();
        WriteStateValues( Statevector );
}


