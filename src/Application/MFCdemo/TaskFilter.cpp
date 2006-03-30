////////////////////////////////////////////////////////////////////////////////
// $Id$
//
// File:    TaskFilter.cpp
//
// Author:  juergen.mellinger@uni-tuebingen.de
//
// Description: A simple task filter that illustrates how to implement an
//          application module with Microsoft's MFC library.
//
// $Log$
// Revision 1.1  2006/03/30 13:49:16  mellinger
// Initial version.
//
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "TaskFilter.h"
#include "UState.h"
#include "UBCIError.h"
#include "MeasurementUnits.h"
#include "UBCITime.h"

using namespace std;

RegisterFilter( TaskFilter, 3 );

TaskFilter::TaskFilter()
{
  BEGIN_PARAMETER_DEFINITIONS
    "UsrTask int WinWidth= 250 0 0 0 // "
      "Window Width in Pixels",
    "UsrTask int WinHeight= 250 0 0 0 // "
      "Window Height in Pixels",
    "UsrTask int WinXPos= 50 0 0 0 // "
      "Window X Location",
    "UsrTask int WinYPos= 50 0 0 0 // "
      "Window Y Location",
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
TaskFilter::Preflight( const SignalProperties& inputProperties,
                                   SignalProperties& outputProperties ) const
{
  PreflightCondition( Parameter( "WindowWidth" ) > 0 );
  PreflightCondition( Parameter( "WindowHeight" ) > 0 );
  // TaskFilter::Process() implies that the input signal has at least two integer channels
  // with one element each.
  PreflightCondition( inputProperties >= SignalProperties( 2, 1, SignalType::int16 ) );
  outputProperties = inputProperties;
}

void
TaskFilter::Initialize()
{
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
TaskFilter::Process( const GenericSignal* Input, GenericSignal* Output )
{
  mWindow.CursorX( mWindow.CursorX() + ( *Input )( 1, 0 ) / 32768 );
  mWindow.CursorY( mWindow.CursorY() + ( *Input )( 0, 0 ) / 32768 );
  mWindow.RedrawWindow();
  State( "StimulusTime" ) = BCITIME::GetBCItime_ms();
  *Output = *Input;
}

void
TaskFilter::StartRun()
{
  mWindow.CursorX( 0.5 );
  mWindow.CursorY( 0.5 );
}

void
TaskFilter::StopRun()
{
  mWindow.CursorX( 0.5 );
  mWindow.CursorY( 0.5 );
}
