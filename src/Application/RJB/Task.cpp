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
#include <cmath>
#include <assert>

using namespace std;

RegisterFilter( TTask, 3 );

TTask::TTask()
: mpUser( new Usr ),
  x_pos( 0 ),
  y_pos( 0 ),
  cursor_x_start( 0 ),
  cursor_y_start( 0 ),
  limit_right( 0 ),
  limit_left( 0 ),
  limit_top( 0 ),
  limit_bottom( 0 ),
  Ntargets( 0 ),
  targetcount( 0 ),
  ranflag( 0 ),
  Hits( 0 ),
  Misses( 0 ),
  OldRunning( 0 ),
  OldCurrentTarget( 0 ),
  HitOrMiss( 0 ),
  randseed( 0 ),
  BaselineInterval( 0 ),
  CurrentTarget( 0 ),
  TargetTime( 0 ),
  TargetDuration( 0 ),
  CurrentOutcome( 0 ),
  OutcomeTime( 0 ),
  OutcomeDuration( 0 ),
  CurrentBaseline( 0 ),
  BaselineTime( 0 ),
  CurrentFeedback( 0 ),
  FeedbackTime( 0 ),
  CurrentIti( 0 ),
  CurrentPri( 0 ),
  CurrentRunning( 0 ),
  CurRunFlag( 0 ),
  CurrentRest( 0 ),
  Resting( 0 ),
  ItiTime( 0 ),
  PriTime( 0 ),
  ItiDuration( 0 ),
  PreRunInterval( 0 ),
  PtpTime( 0 ),
  PtpDuration( 0 ),
  runstarttime( 0 ),
  timelimit( 0 ),
  trial( 0 ),
  run( 0 ),
  timepassed( 0 ),
  mTaskLog( ".apl" ),
  mVis( SOURCEID::TASKLOG )
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
    "UsrTask int BaselineInterval= 1 0 0 2 // "
        "Intercept Computation 0 = none 1 = targets 2 = ITI (enumeration)",
    "UsrTask int TimeLimit= 180 180 0 1000 // "
        "Time Limit for Runs in seconds",
    "UsrTask int RestingPeriod= 0 0 0 1 // "
        "rest period of data acquisition (boolean)",
    "UsrTask matrix Announcements= "
      " { 0 1 2 } { Task Result Cursor Hit Miss } "
      "                %      %      %   %    % "
      "                %      %      %   %    % "
      "                %      %      %   %    % "
      "// Sound files for auditory announcements",
    "UsrTask matrix AnnouncementVolumes= "
      " { 0 1 2 } { Task Result Cursor Hit Miss } "
      "                1      1      1   1    1 "
      "                1      1      1   1    1 "
      "                1      1      1   1    1 "
      " 1.0 0.0 1.0 // Volume settings for announcements",
    "UsrTask matrix CursorVolume= "
      " { 0 1 2 } { min max } "
      " 0.5 1.0 "
      " 0.5 1.0 "
      " 0.5 1.0 "
      " 1.0 1.0 0.0 1.0 // Volume ranges for cursor announcements",
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
    "Standby 1 0 0 0",
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
}

//-----------------------------------------------------------------------------

TTask::~TTask( void )
{
  delete mpUser;
}

void
TTask::Preflight( const SignalProperties& inputProperties,
                        SignalProperties& outputProperties ) const
{
  TWavePlayer preflightPlayer;
  bool        checkedVolumeAdjustment = true;
  for( size_t i = 0; i < Parameter( "Announcements" )->GetNumValuesDimension1(); ++i )
    for( size_t j = 0; j < Parameter( "Announcements" )->GetNumValuesDimension2(); ++j )
  {
    string soundFile = string( Parameter( "Announcements", i, j ) );
    if( soundFile != "" )
    {
      if( preflightPlayer.AttachFile( soundFile.c_str() ) != TWavePlayer::noError )
        bcierr << "Could not open " << soundFile << " for audio playback" << endl;
      if( !checkedVolumeAdjustment
          && Parameter( "Announcements" )->ColumnLabels()[ j ] == "Cursor" )
      {
        if( preflightPlayer.SetVolume( Parameter( "CursorVolume" ) ) != TWavePlayer::noError )
          bcierr << "Could not adjust volume for audio output device"
                 << " -- cursor announcement will not work properly"
                 << endl;
        checkedVolumeAdjustment = true;
      }
    }
  }
  mTaskLog.Preflight();
  // TTask::Process() implies that the input signal has at least two integer channels
  // with one element each.
  PreflightCondition( inputProperties >= SignalProperties( 2, 1, SignalType::int16 ) );
  outputProperties = inputProperties;
}

void TTask::Initialize()
{
  PreRunInterval =   MeasurementUnits::ReadAsTime( Parameter( "PreRunInterval" ) );
  PtpDuration =      MeasurementUnits::ReadAsTime( Parameter( "PreTrialPause" ) );
  ItiDuration =      MeasurementUnits::ReadAsTime( Parameter( "ItiDuration" ) );
  OutcomeDuration =  MeasurementUnits::ReadAsTime( Parameter( "RewardDuration" ) );
  Ntargets =         Parameter( "NumberTargets" );
  BaselineInterval = Parameter( "BaselineInterval" );
  Resting =          Parameter( "RestingPeriod" );

  timelimit =        Parameter( "TimeLimit" );

  mTaskAnnouncements.resize( Ntargets + 1 );
  mResultAnnouncements.resize( Ntargets + 1 );
  mCursorAnnouncements.resize( Ntargets + 1 );
  mCursorVolumeOffsets.resize( Ntargets + 1 );
  mCursorVolumeSlopes.resize( Ntargets + 1 );
  mHitAnnouncements.resize( Ntargets + 1 );
  mMissAnnouncements.resize( Ntargets + 1 );
  for( int i = 0; i <= Ntargets; ++i )
  {
    mTaskAnnouncements[ i ].AttachFile( Parameter( "Announcements", i, "Task" ) );
    mTaskAnnouncements[ i ].SetVolume( Parameter( "AnnouncementVolumes", i, "Task" ) );

    mResultAnnouncements[ i ].AttachFile( Parameter( "Announcements", i, "Result" ) );
    mResultAnnouncements[ i ].SetVolume( Parameter( "AnnouncementVolumes", i, "Result" ) );

    mCursorAnnouncements[ i ].AttachFile( Parameter( "Announcements", i, "Cursor" ) );
    mCursorVolumeOffsets[ i ] = Parameter( "CursorVolume", i, "min" );
    mCursorVolumeOffsets[ i ] *= Parameter( "AnnouncementVolumes", i, "Cursor" );
    mCursorVolumeSlopes[ i ] = Parameter( "CursorVolume", i, "max" );
    mCursorVolumeSlopes[ i ] *= Parameter( "AnnouncementVolumes", i, "Cursor" );
    mCursorVolumeSlopes[ i ] -= mCursorVolumeOffsets[ i ];

    mHitAnnouncements[ i ].AttachFile( Parameter( "Announcements", i, "Hit" ) );
    mHitAnnouncements[ i ].SetVolume( Parameter( "AnnouncementVolumes", i, "Hit" ) );

    mMissAnnouncements[ i ].AttachFile( Parameter( "Announcements", i, "Miss" ) );
    mMissAnnouncements[ i ].SetVolume( Parameter( "AnnouncementVolumes", i, "Miss" ) );
  }

  mTaskLog.Initialize();
  bitrate.Initialize(Ntargets);

  trial=1;

  mVis.Send( CFGID::WINDOWTITLE, "User Task Log" );

  mpUser->Initialize();
  mpUser->GetLimits( &limit_right, &limit_left, &limit_top, &limit_bottom );
  targetcount= 0;
  ranflag= 0;

  time_t ctime= time(NULL);
  time( &ctime );
  randseed= -ctime;

  cursor_x_start= limit_left;
  cursor_y_start= ( limit_top + limit_bottom ) / 2;

  ReadStateValues();

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
  mpUser->PutO(false);

  WriteStateValues();
}

void TTask::ReadStateValues()
{
  CurrentTarget = State( "TargetCode" );
  CurrentOutcome = State( "ResultCode" );
  CurrentFeedback = State( "Feedback" );
  CurrentIti = State( "IntertrialInterval" );
  CurrentRunning = State( "Running" );
  if( CurRunFlag == 1 )     // 0 must cycle through at least once
  {
    if( CurrentRunning == 1 )
      CurrentRunning = 0;
    else
      CurRunFlag= 0;
  }
  CurrentRest = State( "RestPeriod" );
}

void TTask::WriteStateValues()
{
  State( "StimulusTime" ) = BCITIME::GetBCItime_ms(); // time stamp
  State( "TargetCode" ) = CurrentTarget;
  State( "ResultCode" ) = CurrentOutcome;
  State( "Feedback" ) = CurrentFeedback;
  State( "IntertrialInterval" ) = CurrentIti;
  State( "Running" ) = CurrentRunning;
  State( "RestPeriod" ) = CurrentRest;
  State( "CursorPosX" ) = int( x_pos * mpUser->scale_x );
  State( "CursorPosY" ) = int( y_pos * mpUser->scale_y );
}

void TTask::ShuffleTargs( int ntargs )
{
  for( int i = 0; i < ntargs; ++i )
  {
    bool goodTarget = false;
    do {
      float rval = mpUser->ran1( &randseed );
      targs[ i ] = 1 + int( rval * ntargs );
      goodTarget = ( targs[ i ] > 0 );
      goodTarget &= ( targs[ i ] <= Ntargets );
      for( int j = 0; goodTarget && j < i; ++j )
        goodTarget &= ( targs[ j ] != targs[ i ] );
    } while( !goodTarget );
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
  CurrentOutcome = mpUser->TestCursorLocation( x, y, CurrentTarget );
  if( CurrentOutcome > 0 )
  {
    if( CurrentOutcome == CurrentTarget )
    {
      Hits++;
      HitOrMiss=true;
    }
    else
    {
      Misses++;
      HitOrMiss=false;
    }
    CurrentFeedback = 0;
    if( BaselineInterval == 1 )
      CurrentBaseline = 0;
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
 if (CurrentRunning == 0)
    CurrentRunning=0;

 if ((CurrentRunning == 0) && (OldRunning == 1))            // put the T up there on the transition from not suspended to suspended
    {
    mpUser->Clear();
    mpUser->HideBackground();
    mpUser->PutT(true);
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
    mpUser->PutT(false);
    // in the ITI period ?
    if (CurrentIti > 0)
       {
       // ITI period just started
       if (ItiTime == 0)
          {
          mVis << "Run: " << run << " ITI -> new trial: " << trial << endl;
          trial++;
          }
            // in case the run was longer than x seconds
            timepassed= ((double)(TDateTime::CurrentTime())-runstarttime)*86400;

            if (timepassed > timelimit)
               {
               CurrentRunning=0;
               CurRunFlag= 1;
               if( Hits + Misses > 0 )
               {
                 mVis << "Run " << run << " - "
                      << fixed << setprecision( 1 ) << Hits * 100. / ( Hits + Misses )
                      << "% correct"
                      << endl;
               }
               else
               {
                  mVis << "No statistics for this run\n"
                       << "There were no trials" << endl;
               }
               mVis << "Total Bits transferred: "
                    << fixed << setprecision( 2 ) << bitrate.TotalBitsTransferred() << "\n"
                    << "Average Bits/Trial: "
                    << fixed << setprecision( 2 ) << bitrate.BitsPerTrial() << "\n"
                    << "Average Bits/Minute: "
                    << fixed << setprecision( 2 ) << bitrate.BitsPerMinute() << "\n"
                    << "**************************" << endl;
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
               mResultAnnouncements[ CurrentOutcome ].Play();
               if( CurrentOutcome == CurrentTarget )
                 mHitAnnouncements[ CurrentTarget ].Play();
               else
                 mMissAnnouncements[ CurrentTarget ].Play();
               mVis << Hits << " hits " << Misses << " missed" << endl;
               bitrate.Push(HitOrMiss);
               }

            }
    }

 // new trial just started
 if ((CurrentTarget > 0) && (OldCurrentTarget == 0))
    mVis << "Target: " << CurrentTarget << endl;

 OldRunning=CurrentRunning;
 OldCurrentTarget=CurrentTarget;
}


// if we have defined this period, show some initial feedback
// after a run has been started
void TTask::Pri( void )
{
 mpUser->PreRunInterval(PriTime);
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
        mpUser->Clear();
        ItiTime++;

        // The "Standby" state will prolong the ITI indefinitely.
        if( !State( "Standby" ) && ItiTime > ItiDuration )
        {
                CurrentIti = 0;
                ItiTime = 0;
                CurrentTarget= GetTargetNo( Ntargets );
                mpUser->PutTarget(CurrentTarget, Usr::TARGET_ON );
                mpUser->ShowBackground();
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
                if( mpUser->scale_x > 0.0 ) x_pos/= mpUser->scale_x;
                if( mpUser->scale_y > 0.0 ) y_pos/= mpUser->scale_y;
                mpUser->PutCursor( x_pos, y_pos, Usr::CURSOR_ON );
                CurrentFeedback= 1;
                PtpTime= 0;
        }

}

void TTask::Feedback( short sig_x, short sig_y )
{
  x_pos += sig_x;
  y_pos += sig_y;
  mpUser->PutCursor( x_pos, y_pos, Usr::CURSOR_ON );
  TestCursorLocation( x_pos, y_pos );
  if( CurrentFeedback )
  {
    float pos = y_pos * mpUser->scale_y;
    if( pos < mpUser->targy[ 1 ] )
      pos = mpUser->targy[ 1 ];
    if( pos > mpUser->targy_btm[ Ntargets ] )
      pos = mpUser->targy_btm[ Ntargets ];
    pos -= mpUser->targy[ 1 ];
    pos /= ( mpUser->targy_btm[ Ntargets ] - mpUser->targy[ 1 ] );
    for( int i = 1; i <= Ntargets; ++i )
    {
      float targetPos = ( i - 1 ) / float( Ntargets - 1 ),
            volume = 1.0;
      if( pos < targetPos )
        volume = mCursorVolumeOffsets[ i ] + mCursorVolumeSlopes[ i ] * ( pos / targetPos );
      else if( pos > targetPos )
        volume = mCursorVolumeOffsets[ i ] + mCursorVolumeSlopes[ i ] * ( 1 - pos ) / ( 1 - targetPos );
      if( volume > 1.0 )
        volume = 1.0;
      else if( volume < 0.0 )
        volume = 0.0;
      mCursorAnnouncements[ i ].SetVolume( volume );
      if( !mCursorAnnouncements[ i ].IsPlaying() )
        mCursorAnnouncements[ i ].Play();
    }
  }
  else
    for( size_t i = 0; i < mCursorAnnouncements.size(); ++i )
      mCursorAnnouncements[ i ].Stop();
}


void TTask::Outcome()
{
        OutcomeTime++;
        mpUser->Outcome( OutcomeTime, CurrentOutcome ); // flash outcome when YES/NO

        if( OutcomeTime >= OutcomeDuration-1 )
        {
                CurrentIti= 1;
                CurrentOutcome= 0;
                CurrentTarget= 0;
                OutcomeTime= 0;
                mpUser->PutCursor( x_pos, y_pos, Usr::CURSOR_OFF );
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
                mpUser->HideBackground();
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
               mpUser->HideBackground();
               mpUser->PutO(false);
               mpUser->PutT(true);
               CurrentRest= 0;
               CurrentIti= 1;
         }

}

void TTask::Process( const GenericSignal* Input, GenericSignal* Output )
{
  ReadStateValues();

  if( CurrentRunning > 0 )
  {
    if( CurrentRest > 0 )
      Rest();
    else if( CurrentPri > 0 )
      Pri();
    else if( CurrentIti > 0 )
      Iti();
    else if( CurrentFeedback > 0 )
      Feedback( ( *Input )( 1, 0 ), ( *Input )( 0, 0 ) );
    else if( CurrentOutcome  > 0 )
      Outcome();
    else if( CurrentTarget   > 0 )
      Ptp();
  }

  UpdateDisplays();
  WriteStateValues();
  *Output = *Input;
}

void TTask::StopRun()
{
  for( size_t i = 0; i < mTaskAnnouncements.size(); ++i )
    mTaskAnnouncements[ i ].Stop();
  for( size_t i = 0; i < mResultAnnouncements.size(); ++i )
    mResultAnnouncements[ i ].Stop();
  for( size_t i = 0; i < mCursorAnnouncements.size(); ++i )
    mCursorAnnouncements[ i ].Stop();
  for( size_t i = 0; i < mHitAnnouncements.size(); ++i )
    mHitAnnouncements[ i ].Stop();
  for( size_t i = 0; i < mMissAnnouncements.size(); ++i )
    mMissAnnouncements[ i ].Stop();
}

