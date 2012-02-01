////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A demo feedback task.
//
// $BEGIN_BCI2000_LICENSE$
// 
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2012: BCI2000 team and many external contributors ]
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

#include "FeedbackDemoTask.h"
#include "Color.h"
#include "Localization.h"
#ifdef __BORLANDC__
#include <vcl.h>
#else // __BORLANDC__
#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsSimpleTextItem>
#endif // __BORLANDC__

RegisterFilter( FeedbackDemoTask, 3 );

using namespace std;

FeedbackDemoTask::FeedbackDemoTask()
: mRunCount( 0 ),
  mTrialCount( 0 ),
  mCursorPosX( 0.0 ),
  mCursorPosY( 0.0 ),
  mCursorSpeedX( 0.0 ),
  mCursorSpeedY( 0.0 ),
#ifdef __BORLANDC__
  mpForm( new TForm( reinterpret_cast<TComponent*>( NULL ) ) ),
  mpLabel( new TLabel( mpForm ) ),
  mpTarget( new TShape( mpForm ) ),
  mpCursor( new TShape( mpForm ) )
#else // __BORLANDC__
  mpForm( NULL ),
  mpScene( NULL ),
  mpSceneView( NULL ),
  mpLabel( NULL ),
  mpTarget( NULL ),
  mpCursor( NULL )
#endif // __BORLANDC__
{
  BEGIN_PARAMETER_DEFINITIONS
    "Application:Window int WindowWidth= 640 640 0 % "
      " // width of application window",
    "Application:Window int WindowHeight= 480 480 0 % "
      " // height of application window",
    "Application:Window int WindowLeft= 0 0 % % "
      " // screen coordinate of application window's left edge",
    "Application:Window int WindowTop= 0 0 % % "
      " // screen coordinate of application window's top edge",

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

#ifdef __BORLANDC__
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
#else // __BORLANDC__
  mpForm = new QWidget;
  mpForm->setWindowFlags( Qt::FramelessWindowHint );
  mpForm->setWindowTitle( "BCI2000 Feedback Demo" );

  mpScene = new QGraphicsScene;
  mpScene->setBackgroundBrush( QBrush( Qt::black ) );

  mpSceneView = new QGraphicsView( mpForm );
  mpSceneView->setScene( mpScene );
  mpSceneView->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  mpSceneView->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  mpSceneView->show();

  mpLabel = new QGraphicsSimpleTextItem( NULL, mpScene );
  mpLabel->show();

  mpTarget = new QGraphicsRectItem( NULL, mpScene );
  mpTarget->setPen( Qt::NoPen );
  mpTarget->hide();

  mpCursor = new QGraphicsEllipseItem( NULL, mpScene );
  mpCursor->setPen( Qt::NoPen );
  mpCursor->hide();

  mpForm->hide();
#endif // __BORLANDC__
}

FeedbackDemoTask::~FeedbackDemoTask()
{
  if( mpForm )
    delete mpForm;
  mpForm = NULL;
#ifndef __BORLANDC__
  if( mpScene )
    delete mpScene;
  mpScene = NULL;
#endif // __BORLANDC__
}

void
FeedbackDemoTask::OnPreflight( const SignalProperties& Input ) const
{
  Parameter( "WindowHeight" );
  Parameter( "WindowWidth" );
  Parameter( "WindowLeft" );
  Parameter( "WindowTop" );

  if( RGBColor( Parameter( "CursorColor" ) ) == RGBColor( RGBColor::NullColor ) )
    bcierr << "Invalid RGB value in CursorColor" << endl;

  if( Parameter( "FeedbackDuration" ).InSampleBlocks() <= 0 )
    bcierr << "FeedbackDuration must be greater 0" << endl;

  if( Input.IsEmpty() )
    bcierr << "Requires at least one entry in control signal" << endl;

  if( Input.Channels() > 1 )
    bciout << "Will ignore additional channels in control signal" << endl;
}

void
FeedbackDemoTask::OnInitialize( const SignalProperties& /*Input*/ )
{
#ifdef __BORLANDC__
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
  float feedbackDuration = Parameter( "FeedbackDuration" ).InSampleBlocks();
  mCursorSpeedX = ( mpForm->Width - mpTarget->Width ) / feedbackDuration;
  // In Y direction, we need only cross half the screen height during a trial.
  mCursorSpeedY = mpForm->Height / feedbackDuration / 2;

  mpForm->Show();
#else // __BORLANDC__
  mpForm->move( Parameter( "WindowLeft" ), Parameter( "WindowTop" ) );
  mpForm->resize( Parameter( "WindowWidth" ), Parameter( "WindowHeight" ) );
  mpSceneView->resize( Parameter( "WindowWidth" ), Parameter( "WindowHeight" ) );
  mpSceneView->setSceneRect( 0, 0, Parameter( "WindowWidth"), Parameter( "WindowHeight" ) );
  mpSceneView->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  mpSceneView->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  mpSceneView->show();

  RGBColor textColor( RGBColor::Green );
  SetLabel( LocalizableString( "Timeout" ), textColor );

  QRectF cursorRect;
  cursorRect.setWidth( mpForm->width() * Parameter( "CursorWidth" ) / 100.0 );
  cursorRect.setHeight( cursorRect.width() );
  RGBColor cursorColor = RGBColor( Parameter( "CursorColor" ) );
  QBrush cursorBrush( QColor( cursorColor.R(), cursorColor.G(), cursorColor.B() ) );
  mpCursor->setRect( cursorRect );
  mpCursor->setBrush( cursorBrush );

  QRectF targetRect;
  targetRect.setLeft( mpForm->width() - mpCursor->rect().width() );
  targetRect.setWidth( mpCursor->rect().width() );
  if( Parameter( "NumberTargets" ) > 0 )
    targetRect.setHeight( mpForm->height() / Parameter( "NumberTargets" ) );
  mpTarget->setRect( targetRect );

  // Cursor speed in pixels per signal block duration:
  float feedbackDuration = Parameter( "FeedbackDuration" ).InSampleBlocks();
  mCursorSpeedX = ( mpForm->width() - mpTarget->rect().width() ) / feedbackDuration;
  // In Y direction, we need only cross half the screen height during a trial.
  mCursorSpeedY = mpForm->height() / feedbackDuration / 2;

  mpForm->show();
#endif // __BORLANDC__
}

void
FeedbackDemoTask::OnStartRun()
{
  ++mRunCount;
  mTrialCount = 0;
  mTrialStatistics.Reset();
  AppLog << "Run #" << mRunCount << " started" << endl;

#ifdef __BORLANDC__
  mpLabel->Color = clWhite;
  mpLabel->Caption = LocalizableString( "Be prepared ..." );
#else // __BORLANDC__
  RGBColor textColor( RGBColor::White );
  SetLabel( LocalizableString( "Be prepared ..." ), textColor );
#endif // __BORLANDC__
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

#ifdef __BORLANDC__
  mpLabel->Font->Color = clGreen;
  mpLabel->Caption = LocalizableString( "Timeout" );
#else // __BORLANDC__
  RGBColor textColor( RGBColor::Green );
  SetLabel( LocalizableString( "Timeout" ), textColor );
#endif // __BORLANDC__
}

void
FeedbackDemoTask::OnTrialBegin()
{
  ++mTrialCount;
  AppLog.Screen << "Trial #" << mTrialCount
                << ", target: " << State( "TargetCode" )
                << endl;

#ifdef __BORLANDC__
  mpLabel->Caption = "";
  if( State( "TargetCode" ) > 0 )
  {
    mpTarget->Top = mpTarget->Height * ( State( "TargetCode" ) - 1 );
    mpTarget->Brush->Color = mpCursor->Brush->Color;
    mpTarget->Show();
  }
#else // __BORLANDC__
  RGBColor textColor( RGBColor::Black );
  SetLabel( "", textColor );
  if( State( "TargetCode" ) > 0 )
  {
    QRectF targetRect = mpTarget->rect();
    QRectF newTargetRect( targetRect.left(), targetRect.height() * ( State ( "TargetCode" ) - 1 ), targetRect.width(), targetRect.height() );
    mpTarget->setRect( newTargetRect );
    mpTarget->setBrush( mpCursor->brush() );
    mpTarget->show();
  }
#endif // __BORLANDC__
}

void
FeedbackDemoTask::OnTrialEnd()
{
#ifdef __BORLANDC__
  mpLabel->Caption = "";
  mpTarget->Hide();
  mpCursor->Hide();
#else //__BORLANDC__
  mpLabel->setText( QString( "" ) );
  mpTarget->hide();
  mpCursor->hide();
#endif // __BORLANDC__
}

void
FeedbackDemoTask::OnFeedbackBegin()
{
  mCursorPosX = 0.0;
#ifdef __BORLANDC__
  mCursorPosY = mpForm->Height / 2;
  mpCursor->Left = mCursorPosX;
  mpCursor->Top = mCursorPosY - mpCursor->Height / 2;
  mpCursor->Show();
#else // __BORLANDC__
  mCursorPosY = mpForm->height() / 2;
  QRectF cursorRect = mpCursor->rect();
  QRectF newCursorRect( mCursorPosX, mCursorPosY - cursorRect.height() / 2, cursorRect.width(), cursorRect.height() );
  mpCursor->setRect( newCursorRect );
  mpCursor->show();
#endif // __BORLANDC__
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
#ifdef __BORLANDC__
      mpTarget->Brush->Color = clYellow;
      mpTarget->Invalidate();
#else // __BORLANDC__
      mpTarget->setBrush( QBrush( Qt::yellow ) );
      mpTarget->update();
#endif // __BORLANDC__
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
#ifdef __BORLANDC__
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
#else // __BORLANDC__
  QRectF cursorRect = mpCursor->rect();
  QRectF newCursorRect;
  newCursorRect.setLeft( mCursorPosX );
  newCursorRect.setTop( mCursorPosY - cursorRect.height() / 2 );
  if( newCursorRect.top() < 0 )
    newCursorRect.setTop( 0 );
  else if( newCursorRect.top() > mpForm->height() - cursorRect.height() )
    newCursorRect.setTop( mpForm->height() - cursorRect.height() );
  newCursorRect.setWidth( cursorRect.width() );
  newCursorRect.setHeight( cursorRect.height() );
  mpCursor->setRect( newCursorRect );

  if( mCursorPosX + cursorRect.width() / 2 >= mpForm->width() - mpTarget->rect().width() )
  { // Right margin hit:
    doProgress = true;
    int result = static_cast<int>( ::floor( mCursorPosY * Parameter( "NumberTargets" ) / mpForm->height() ) + 1 );
    if( result < 1 )
      result = 1;
    else if( result > Parameter( "NumberTargets" ) )
      result = Parameter( "NumberTargets" );
    State( "ResultCode" ) = result;
  }
#endif // __BORLANDC__
}

void
FeedbackDemoTask::DoPostFeedback( const GenericSignal&, bool& /*doProgress*/ )
{
}

void
FeedbackDemoTask::DoITI( const GenericSignal&, bool& /*doProgress*/ )
{
}

#ifndef __BORLANDC__
void
FeedbackDemoTask::SetLabel( const char *text, RGBColor &color )
{
  QFont labelFont;
  labelFont.fromString( "Arial" );
  labelFont.setPixelSize( mpForm->height() / 8 );
  QFontMetrics fm( labelFont );
  mpLabel->setFont( labelFont );
  mpLabel->setPos( ( mpForm->width() / 2 ) - ( fm.width( text ) / 2 ),
                   ( mpForm->height() / 2 ) - ( fm.height() / 2 ) );
  mpLabel->setPen( QPen( QColor( color.R(), color.G(), color.B() ) ) );
  mpLabel->setBrush( QBrush( QColor( color.R(), color.G(), color.B() ) ) );
  mpLabel->setText( text );
}
#endif // __BORLANDC__


