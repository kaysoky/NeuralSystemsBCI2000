////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A base class for application modules.
//         This class defines parameters common to all application modules, and
//         defines two output streams intended for logging purposes:
//         - The AppLog stream is directed into a window displayed to the
//           operator user.
//         - The AppLogFile stream is directed into a log file, and displayed
//           in the operator user's log window.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "ApplicationBase.h"

using namespace std;

ApplicationBase::ApplicationBase( const GUI::GraphDisplay* inDisplay )
/*: mDisplayVis( inDisplay ) */
{
  AppLog.Screen.Send( CfgID::WindowTitle, "Application Log" );

  BEGIN_PARAMETER_DEFINITIONS
    "Application:Window int WindowWidth= 640 640 0 % "
      " // width of application window",
    "Application:Window int WindowHeight= 480 480 0 % "
      " // height of application window",
    "Application:Window int WindowLeft= 0 0 % % "
      " // screen coordinate of application window's left edge",
    "Application:Window int WindowTop= 0 0 % % "
      " // screen coordinate of application window's top edge",
  END_PARAMETER_DEFINITIONS

  BEGIN_STATE_DEFINITIONS
    "StimulusTime 16 0 0 0",
  END_STATE_DEFINITIONS
}

int
ApplicationBase::StreamBundle::BundleStringbuf::sync()
{
  int result = stringbuf::sync();
  for( StreamSet::const_iterator i = mStreams.begin(); i != mStreams.end(); ++i )
    ( **i << this->str() ).flush();
  str( "" );
  return result;
}


ApplicationBase::DisplayVisualization::DisplayVisualization( const GUI::GraphDisplay* inDisplay )
: mpDisplay( inDisplay ),
  mVis( "APSC" ),
  mDoVisualize( false ),
  mWidth( 0 ),
  mHeight( 0 ),
  mTemporalDecimation( 1 ),
  mBlockCount( 0 )
{
  if( mpDisplay != NULL )
  {
    BEGIN_PARAMETER_DEFINITIONS
      "Visualize:Application int VisualizeApplicationWindow= 0 0 0 1 "
        "// Display miniature copy of application window (boolean)",
      "Visualize:Application int AppWindowSpatialDecimation= 16 16 1 % "
        "// Application window decimation (shrinking) factor",
      "Visualize:Application int AppWindowTemporalDecimation= 16 16 1 % "
        "// Application window time decimation factor",
    END_PARAMETER_DEFINITIONS
  }
}

void
ApplicationBase::DisplayVisualization::Preflight() const
{
}

void
ApplicationBase::DisplayVisualization::Initialize()
{
  mDoVisualize = ( mpDisplay != NULL ) && ( Parameter( "VisualizeApplicationWindow" ) > 0 );
  if( mDoVisualize )
  {
    mTemporalDecimation = Parameter( "AppWindowTemporalDecimation" );
    int applicationWindowDecimation = Parameter( "AppWindowSpatialDecimation" );
    mWidth = ( mpDisplay->Context().rect.right - mpDisplay->Context().rect.left )
               / applicationWindowDecimation;
    mHeight = ( mpDisplay->Context().rect.bottom - mpDisplay->Context().rect.top )
                / applicationWindowDecimation;

    mVis.Send( BitmapImage( 0, 0 ) ); // An empty image indicates that the next
                                      // image is not a difference image
    mImageBuffer = mpDisplay->BitmapData( mWidth, mHeight );
    mVis.Send( mImageBuffer );
  }
}

void
ApplicationBase::DisplayVisualization::StartRun()
{
  if( mDoVisualize )
  {
    mVis.Send( BitmapImage( 0, 0 ) );
    mImageBuffer.SetBlack();
    mBlockCount = 0;
  }
}

void
ApplicationBase::DisplayVisualization::StopRun()
{
}

void
ApplicationBase::DisplayVisualization::Process()
{
  if( mDoVisualize && ( ++mBlockCount %= mTemporalDecimation ) == 0 )
  {
    BitmapImage curImage = mpDisplay->BitmapData( mWidth, mHeight );
    mVis.Send( curImage - mImageBuffer );
    mImageBuffer = curImage;
  }
}


