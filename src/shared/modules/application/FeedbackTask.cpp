////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A base class for application modules that provide feedback in a
//   trial-based paradigm.
//   Inheriting from ApplicationBase, descendants of FeedbackTask have access
//   to the AppLog, AppLog.File and AppLog.Screen streams declared in
//   ApplicationBase.
//
//   This class performs sequencing, and dispatches GenericFilter::Process()
//   calls to its virtual member functions, depending on the current trial
//   phase. Child classes (descendants) of FeedbackTask implement event
//   handlers by overriding its virtual functions.
//
// $BEGIN_BCI2000_LICENSE$
// 
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2011: BCI2000 team and many external contributors ]
// 
// BCI2000 is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
// 
// BCI2000 is distributed in the hope that it will be useful, but
//                         WITHOUT ANY WARRANTY
// - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.
// 
// $END_BCI2000_LICENSE$
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "FeedbackTask.h"
#include "MeasurementUnits.h"
#include "PrecisionTime.h"

using namespace std;

#ifdef _MSC_VER
#define __FUNC__ __FUNCTION__
#endif // _MSC_VER

FeedbackTask::FeedbackTask( const GUI::GraphDisplay* inDisplay )
: ApplicationBase( inDisplay ),
  mPhase( none ),
  mBlocksInPhase( 0 ),
  mBlocksInRun( 0 ),
  mPreFeedbackDuration( 0 ),
  mFeedbackDuration( 0 ),
  mPostFeedbackDuration( 0 ),
  mITIDuration( 0 ),
  mCurrentTrial( 0 ),
  mNumberOfTrials( 0 ),
  mMinRunLength( 0 ),
  mBlockRandSeq( RandomNumberGenerator )
{
  BEGIN_PARAMETER_DEFINITIONS
   "Application:Sequencing float PreRunDuration= 2s 2s 0 % "
     " // duration of pause preceding first trial",
   "Application:Sequencing float PreFeedbackDuration= 2s 2s 0 % "
     " // duration of target display prior to feedback",
   "Application:Sequencing float FeedbackDuration= 3s 3s 0 % "
     " // duration of feedback",
   "Application:Sequencing float PostFeedbackDuration= 1s 1s 0 % "
     " // duration of result display after feedback",
   "Application:Sequencing float ITIDuration= 1s 1s 0 % "
     " // duration of inter-trial interval",
   "Application:Sequencing float MinRunLength= 120s 120s 0 % "
     " // minimum duration of a run; if blank, NumberOfTrials is used",
   "Application:Sequencing int NumberOfTrials= % 0 0 % "
     " // number of trials; if blank, MinRunLength is used",
   "Application:Targets int NumberTargets= 2 2 0 255 "
     " // number of targets",
  END_PARAMETER_DEFINITIONS

  BEGIN_STATE_DEFINITIONS
   "TargetCode 8 0 0 0",
   "ResultCode 8 0 0 0",
   "Feedback   1 0 0 0",
  END_STATE_DEFINITIONS
}

FeedbackTask::~FeedbackTask()
{
  Halt();
}

void
FeedbackTask::Preflight( const SignalProperties& Input, SignalProperties& Output ) const
{
  OptionalParameter( "RandomSeed" );
  State( "Running" );

  if (!string(Parameter("MinRunLength" )).empty() && !string(Parameter("NumberOfTrials")).empty())
    bcierr << "Either MinRunLength or NumberOfTrials must be blank" << endl;

  bcidbg( 2 ) << "Event: Preflight" << endl;
  OnPreflight( Input );
  Output = Input;
}

void
FeedbackTask::Initialize( const SignalProperties& Input, const SignalProperties& /*Output*/ )
{
  mBlockRandSeq.SetBlockSize( Parameter( "NumberTargets" ) );

  mNumberOfTrials = 0;
  if(!string(Parameter("NumberOfTrials")).empty())
    mNumberOfTrials = Parameter("NumberOfTrials");

  mPreRunDuration = static_cast<int>( MeasurementUnits::ReadAsTime( Parameter( "PreRunDuration" ) ) );
  mPreFeedbackDuration = static_cast<int>( MeasurementUnits::ReadAsTime( Parameter( "PreFeedbackDuration" ) ) );
  mFeedbackDuration = static_cast<int>( MeasurementUnits::ReadAsTime( Parameter( "FeedbackDuration" ) ) );
  mPostFeedbackDuration = static_cast<int>( MeasurementUnits::ReadAsTime( Parameter( "PostFeedbackDuration" ) ) );
  mITIDuration = static_cast<int>( MeasurementUnits::ReadAsTime( Parameter( "ITIDuration" ) ) );
  mMinRunLength = static_cast<long long>( MeasurementUnits::ReadAsTime( Parameter( "MinRunLength" ) ) );
  bcidbg( 2 ) << "Event: Initialize" << endl;
  OnInitialize( Input );
}

void
FeedbackTask::StartRun()
{
  mBlocksInRun = 0;
  mBlocksInPhase = 0;
  mCurrentTrial = 0;
  mPhase = preRun;
  bcidbg( 2 ) << "Event: StartRun" << endl;
  OnStartRun();
}

void
FeedbackTask::StopRun()
{
  if( State( "Feedback" ) )
  {
    bcidbg( 2 ) << "Event: FeedbackEnd" << endl;
    OnFeedbackEnd();
    State( "Feedback" ) = false;
  }
  if( State( "TargetCode" ) != 0 )
  {
    bcidbg( 2 ) << "Event: TrialEnd" << endl;
    OnTrialEnd();
    State( "TargetCode" ) = 0;
  }
  State( "ResultCode" ) = 0;
  mPhase = none;

  bcidbg( 2 ) << "Event: StopRun" << endl;
  OnStopRun();
}

void
FeedbackTask::Halt()
{
  bcidbg( 2 ) << "Event: Halt" << endl;
  OnHalt();
}

void
FeedbackTask::Process( const GenericSignal& Input, GenericSignal& Output )
{
  bool doProgress = true;
  while( doProgress )
  {
    switch( mPhase )
    {
      case preRun:
        doProgress = ( mBlocksInPhase >= mPreRunDuration );
        DoPreRun( Input, doProgress );
        break;

      case preFeedback:
        doProgress = ( mBlocksInPhase >= mPreFeedbackDuration );
        DoPreFeedback( Input, doProgress );
        break;

      case feedback:
        doProgress = ( mBlocksInPhase >= mFeedbackDuration );
        bcidbg( 3 ) << "Feedback Signal: " << Input << endl;
        DoFeedback( Input, doProgress );
        break;

      case postFeedback:
        doProgress = ( mBlocksInPhase >= mPostFeedbackDuration );
        DoPostFeedback( Input, doProgress );
        break;

      case ITI:
        doProgress = ( mBlocksInPhase >= mITIDuration );
        DoITI( Input, doProgress );
        break;

      case postRun:
        doProgress = false;
        break;

      default:
        string fun( __FUNC__ );
        string err( ": Unknown phase value" );
        fun.append( err );
        throw fun.c_str();
    }
    if( doProgress )
    {
      mBlocksInPhase = 0;
      switch( mPhase )
      {
        case preRun:
        case ITI:
          State( "TargetCode" ) = mBlockRandSeq.NextElement();
          ++mCurrentTrial;
          bcidbg( 2 ) << "Event: TrialBegin" << endl;
          OnTrialBegin();
          mPhase = preFeedback;
          break;

        case preFeedback:
          State( "Feedback" ) = true;
          bcidbg( 2 ) << "Event: FeedbackBegin" << endl;
          OnFeedbackBegin();
          mPhase = feedback;
          doProgress = false; // The feedback phase lasts at least one block.
          break;

        case feedback:
          State( "Feedback" ) = false;
          bcidbg( 2 ) << "Event: FeedbackEnd" << endl;
          OnFeedbackEnd();
          mPhase = postFeedback;
          break;

        case postFeedback:
        {
          bcidbg( 2 ) << "Event: TrialEnd" << endl;
          OnTrialEnd();
          State( "TargetCode" ) = 0;
          State( "ResultCode" ) = 0;
          bcidbg( 3 ) << "Blocks in Run: " << mBlocksInRun << "/" << mMinRunLength << endl;
          bool runFinished = false;
          if( mNumberOfTrials > 0 )
            runFinished = ( mCurrentTrial >= mNumberOfTrials );
          else
            runFinished = ( mBlocksInRun >= mMinRunLength );
          if( runFinished )
          {
            doProgress = false;
            State( "Running" ) = false;
            mPhase = postRun;
          }
          else
          {
            mPhase = ITI;
          }
          break;
        }
        default:
          string fun( __FUNC__ );
          string err( ": Unknown phase value" );
          fun.append( err );
          throw fun.c_str();
      }
    }
  }
  ++mBlocksInRun;
  ++mBlocksInPhase;
  State( "StimulusTime" ) = PrecisionTime::Now();
  Output = Input;
}
