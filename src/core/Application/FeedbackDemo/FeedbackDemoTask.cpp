////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A demo feedback task.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "FeedbackDemoTask.h"
#include "Color.h"
#include "MeasurementUnits.h"
#include "Localization.h"
#include <vcl.h>

RegisterFilter( FeedbackDemoTask, 3 );

using namespace std;

FeedbackDemoTask::FeedbackDemoTask()
: mRunCount( 0 ),
  mTrialCount( 0 ),
  mCursorPosX( 0.0 ),
  mCursorPosY( 0.0 ),
  mCursorSpeedX( 0.0 ),
  mCursorSpeedY( 0.0 ),
  mpForm( new TForm( reinterpret_cast<TComponent*>( NULL ) ) ),
  mpLabel( new TLabel( mpForm ) ),
  mpTarget( new TShape( mpForm ) ),
  mpCursor( new TShape( mpForm ) )
{
  BEGIN_PARAMETER_DEFINITIONS
    "Application:Cursor int CursorColor= 0xFF0000 0xFF0000 % % "
      " // feedback cursor color (color)",
    "Application:Cursor float CursorWidth= 5 5 0.0 % "
      " // feedback cursor width in percent of screen width",
  END_PARAMETER_DEFINITIONS

  LANGUAGES "German",
  BEGIN_LOCALIZED_STRINGS
   "Timeout",          "Inaktiv",
   "Be prepared ...",  "Achtung ...",
  END_LOCALIZED_STRINGS

  mpForm->BorderStyle = bsNone;
  mpForm->Color = clBlack;
  mpForm->DoubleBuffered = true;

  mpLabel->Parent = mpForm;
  mpLabel->Align = alClient;
  mpLabel->Alignment = taCenter;
  mpLabel->Layout = tlCenter;
  mpLabel->WordWrap = true;
  mpLabel->Font->Name = "Arial";
  mpLabel->Transparent = true;

  mpTarget->Parent = mpForm;
  mpTarget->Shape = stRectangle;
  mpTarget->Pen->Width = 0;
  mpTarget->Hide();

  mpCursor->Parent = mpForm;
  mpCursor->Shape = stCircle;
  mpCursor->Pen->Width = 0;
  mpCursor->Hide();
}

FeedbackDemoTask::~FeedbackDemoTask()
{
  delete mpForm;
}

void
FeedbackDemoTask::OnPreflight( const SignalProperties& Input ) const
{
  Parameter( "WindowHeight" );
  Parameter( "WindowWidth" );
  Parameter( "WindowLeft" );
  Parameter( "WindowTop" );

  if( RGBColor( Parameter( "CursorColor" ) ) == RGBColor::NullColor )
    bcierr << "Invalid RGB value in CursorColor" << endl;

  if( MeasurementUnits::ReadAsTime( Parameter( "FeedbackDuration" ) ) <= 0 )
    bcierr << "FeedbackDuration must be greater 0" << endl;

  if( Input.IsEmpty() )
    bcierr << "Requires at least one entry in control signal" << endl;

  if( Input.Channels() > 1 )
    bciout << "Will ignore additional channels in control signal" << endl;
}

void
FeedbackDemoTask::OnInitialize( const SignalProperties& /*Input*/ )
{
  mpForm->Height = Parameter( "WindowHeight" );
  mpForm->Width = Parameter( "WindowWidth" );
  mpForm->Left = Parameter( "WindowLeft" );
  mpForm->Top = Parameter( "WindowTop" );

  mpLabel->Font->Size = - mpForm->Height / 8;
  mpLabel->Font->Color = clGreen;
  mpLabel->Caption = LocalizableString( "Timeout" );

  mpCursor->Width = mpForm->Width * Parameter( "CursorWidth" ) / 100.0;
  mpCursor->Height = mpCursor->Width;
  mpCursor->Brush->Color = TColor( RGBColor( Parameter( "CursorColor" ) ).ToWinColor() );

  mpTarget->Width = mpCursor->Width;
  if( Parameter( "NumberTargets" ) > 0 )
    mpTarget->Height = mpForm->Height / Parameter( "NumberTargets" );
  mpTarget->Left = mpForm->Width - mpTarget->Width;

  // Cursor speed in pixels per signal block duration:
  float feedbackDuration = MeasurementUnits::ReadAsTime( Parameter( "FeedbackDuration" ) );
  mCursorSpeedX = ( mpForm->Width - mpTarget->Width ) / feedbackDuration;
  // In Y direction, we need only cross half the screen height during a trial.
  mCursorSpeedY = mpForm->Height / feedbackDuration / 2;

  mpForm->Show();
}

void
FeedbackDemoTask::OnStartRun()
{
  ++mRunCount;
  mTrialCount = 0;
  mTrialStatistics.Reset();
  AppLog << "Run #" << mRunCount << " started" << endl;

  mpLabel->Color = clWhite;
  mpLabel->Caption = LocalizableString( "Be prepared ..." );
}

void
FeedbackDemoTask::OnStopRun()
{
  AppLog   << "Run " << mRunCount << " finished: "
           << mTrialStatistics.Total() << " trials, "
           << mTrialStatistics.Hits() << " hits, "
           << mTrialStatistics.Invalid() << " invalid.\n";
  int validTrials = mTrialStatistics.Total() - mTrialStatistics.Invalid();
  if( validTrials > 0 )
    AppLog << ( 200 * mTrialStatistics.Hits() + 1 ) / validTrials / 2  << "% correct, "
           << mTrialStatistics.Bits() << " bits transferred.\n";
  AppLog   << "====================="  << endl;

  mpLabel->Font->Color = clGreen;
  mpLabel->Caption = LocalizableString( "Timeout" );
}

void
FeedbackDemoTask::OnTrialBegin()
{
  ++mTrialCount;
  AppLog.Screen << "Trial #" << mTrialCount
                << ", target: " << State( "TargetCode" )
                << endl;

  mpLabel->Caption = "";
  if( State( "TargetCode" ) > 0 )
  {
    mpTarget->Top = mpTarget->Height * ( State( "TargetCode" ) - 1 );
    mpTarget->Brush->Color = mpCursor->Brush->Color;
    mpTarget->Show();
  }
}

void
FeedbackDemoTask::OnTrialEnd()
{
  mpLabel->Caption = "";
  mpTarget->Hide();
  mpCursor->Hide();
}

void
FeedbackDemoTask::OnFeedbackBegin()
{
  mCursorPosX = 0.0;
  mCursorPosY = mpForm->Height / 2;
  mpCursor->Left = mCursorPosX;
  mpCursor->Top = mCursorPosY - mpCursor->Height / 2;
  mpCursor->Show();
}

void
FeedbackDemoTask::OnFeedbackEnd()
{
  if( State( "ResultCode" ) == 0 )
  {
    AppLog.Screen << "-> aborted" << endl;
    mTrialStatistics.UpdateInvalid();
  }
  else
  {
    mTrialStatistics.Update( State( "TargetCode" ), State( "ResultCode" ) );
    if( State( "TargetCode" ) == State( "ResultCode" ) )
    {
      mpTarget->Brush->Color = clYellow;
      mpTarget->Invalidate();
      AppLog.Screen << "-> hit" << endl;
    }
    else
    {
      AppLog.Screen << "-> miss" << endl;
    }
  }
}

void
FeedbackDemoTask::DoPreRun( const GenericSignal&, bool& /*doProgress*/ )
{
}

void
FeedbackDemoTask::DoPreFeedback( const GenericSignal&, bool& /*doProgress*/ )
{
}

void
FeedbackDemoTask::DoFeedback( const GenericSignal& ControlSignal, bool& doProgress )
{
  mCursorPosX += mCursorSpeedX;
  mCursorPosY -= mCursorSpeedY * ControlSignal( 0, 0 );
  mpCursor->Left = mCursorPosX;
  mpCursor->Top = mCursorPosY - mpCursor->Height / 2;
  if( mpCursor->Top < 0 )
    mpCursor->Top = 0;
  else if( mpCursor->Top > mpForm->Height - mpCursor->Height )
    mpCursor->Top = mpForm->Height - mpCursor->Height;

  if( mCursorPosX + mpCursor->Width / 2 >= mpForm->Width - mpTarget->Width )
  { // Right margin hit:
    doProgress = true;
    int result = ::floor( mCursorPosY * Parameter( "NumberTargets" ) / mpForm->Height ) + 1;
    if( result < 1 )
      result = 1;
    else if( result > Parameter( "NumberTargets" ) )
      result = Parameter( "NumberTargets" );
    State( "ResultCode" ) = result;
  }
}

void
FeedbackDemoTask::DoPostFeedback( const GenericSignal&, bool& /*doProgress*/ )
{
}

void
FeedbackDemoTask::DoITI( const GenericSignal&, bool& /*doProgress*/ )
{
}


