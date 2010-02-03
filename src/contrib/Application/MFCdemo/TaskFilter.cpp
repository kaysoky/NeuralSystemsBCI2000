////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author:  juergen.mellinger@uni-tuebingen.de
// Description: A simple task filter that illustrates how to implement a
//   BCI2000 application module based on Microsoft's MFC library.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "TaskFilter.h"
#include "BCIError.h"
#include "MeasurementUnits.h"
#include "PrecisionTime.h"

using namespace std;

RegisterFilter( TaskFilter, 3 );

TaskFilter::TaskFilter()
: mCursorSpeed( 0 )
{
  BEGIN_PARAMETER_DEFINITIONS
	  "Application:Window int WinWidth= 250 0 0 % // "
      "Window Width in Pixels",
    "Application:Window int WinHeight= 250 0 0 % // "
      "Window Height in Pixels",
    "Application:Window int WinXPos= 50 0 0 % // "
      "Window X Location",
    "Application:Window int WinYPos= 50 0 0 % // "
      "Window Y Location",
	"Application:Feedback int FeedbackDuration= 2s % % % // " 
	  "Duration of a feedback trial",
  END_PARAMETER_DEFINITIONS

  BEGIN_STATE_DEFINITIONS
    "TargetCode 5 0 0 0",
    "ResultCode 5 0 0 0",
    "Feedback 2 0 0 0",
    "IntertrialInterval 2 0 0 0",
    "StimulusTime 16 0 0 0",
  END_STATE_DEFINITIONS

  mWindow.Create( CMFCdemoDlg::IDD );
}

TaskFilter::~TaskFilter( void )
{
  mWindow.DestroyWindow();
}

void
TaskFilter::Preflight( const SignalProperties& Input,
                             SignalProperties& Output ) const
{
  PreflightCondition( Parameter( "FeedbackDuration" ) > 0.0 );
  Output = Input;
}

void
TaskFilter::Initialize( const SignalProperties& Input,
					    const SignalProperties& Output )
{
	
  mCursorSpeed = 1.0 / MeasurementUnits::ReadAsTime( Parameter( "FeedbackDuration" ) );
  mWindow.MoveWindow(
    Parameter( "WinXPos" ),
    Parameter( "WinYPos" ),
    Parameter( "WinWidth" ),
    Parameter( "WinHeight" ),
    false
  );
  mWindow.ShowWindow( SW_SHOWNORMAL );
}


void
TaskFilter::Process( const GenericSignal& Input, GenericSignal& Output )
{
  if( Input.Channels() > 0 )
  {
    float cursorX = mWindow.CursorX() + mCursorSpeed * Input( 0, 0 );
	mWindow.SetCursorX( cursorX - ::floor( cursorX ) );
  }
  if( Input.Channels() > 1 )
  {
    float cursorY = mWindow.CursorY() + mCursorSpeed * Input( 1, 0 );
	mWindow.SetCursorY( cursorY - ::floor( cursorY ) );
  }
  mWindow.RedrawWindow();
  State( "StimulusTime" ) = PrecisionTime::Now();
  Output = Input;
}

void
TaskFilter::StartRun()
{
  mWindow.SetCursorX( 0.5 );
  mWindow.SetCursorY( 0.5 );
  mWindow.RedrawWindow();
}

void
TaskFilter::StopRun()
{
  mWindow.SetCursorX( 0.5 );
  mWindow.SetCursorY( 0.5 );
  mWindow.RedrawWindow();
}
