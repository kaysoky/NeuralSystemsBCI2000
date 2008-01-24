////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: The CursorFeedback Application's Task filter.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "CursorFeedbackTask.h"
#include "MeasurementUnits.h"
#include "Localization.h"
#include "BCIDirectory.h"

// These two lines should be kept consistent:
#define CURSOR_POS_BITS   "13"
const int cCursorPosBits = 13;

RegisterFilter( CursorFeedbackTask, 3 );

using namespace std;

CursorFeedbackTask::CursorFeedbackTask()
: FeedbackTask( &mWindow ),
  mpMessage( NULL ),
  mpScene( NULL ),
  mpCursor( NULL ),
  mpBoundary( NULL ),
  mRunCount( 0 ),
  mTrialCount( 0 ),
  mCurFeedbackDuration( 0 ),
  mMaxFeedbackDuration( 0 ),
  mCursorSpeedX( 1.0 ),
  mCursorSpeedY( 1.0 ),
  mCursorSpeedZ( 1.0 ),
  mCursorColorFront( RGBColor::Red ),
  mCursorColorBack( RGBColor::Red )
{
  BEGIN_PARAMETER_DEFINITIONS
    "Application:Window int RenderingQuality= 1 0 0 1 "
      " // rendering quality: 0: low, 1: high (enumeration)",
    "Application:Window int WindowBitDepth= 16 16 1 32 "
      " // color bit depth of feedback window",

    "Application:Sequencing float MaxFeedbackDuration= 3s % 0 % "
      " // abort a trial after this amount of feedback time has expired",

    "Application:3DEnvironment floatlist CameraPos= 3 50 50 150 % % "
      " // camera position vector in percent coordinates of 3D area",
    "Application:3DEnvironment floatlist CameraAim= 3 50 50 50 % % "
      " // camera aim point in percent coordinates",
    "Application:3DEnvironment int CameraProjection= 0 0 0 2 "
      " // projection type: 0: flat, 1: wide angle perspective, 2: narrow angle perspective (enumeration)",
    "Application:3DEnvironment floatlist LightSourcePos= 3 50 50 100 % % "
      " // light source position in percent coordinates",
    "Application:3DEnvironment int LightSourceColor= 0x808080 % % "
      " // light source RGB color (color)",
    "Application:3DEnvironment int WorkspaceBoundaryColor= 0xffffff 0 % % "
      " // workspace boundary color (0xff000000 for invisible) (color)",
    "Application:3DEnvironment string WorkspaceBoundaryTexture= images/grid.bmp % % % "
      " // path of workspace boundary texture (inputfile)",

    "Application:Cursor float CursorWidth= 10 10 0.0 % "
      " // feedback cursor width in percent of screen width",
    "Application:Cursor int CursorColorFront= 0xff0000 % % % "
       " // cursor color when it is at the front of the workspace (color)",
    "Application:Cursor int CursorColorBack= 0xffff00 % % % "
       " // cursor color when it is in the back of the workspace (color)",
    "Application:Cursor string CursorTexture= % % % %"
      " // path of cursor texture (inputfile)",
    "Application:Cursor floatlist CursorPos= 3 50 50 50 % % "
      " // cursor starting position",

    "Application:Targets matrix Targets= "
      " 5 " // rows
      " [pos%20x pos%20y pos%20z width%20x width%20y width%20z] " // columns
      "  35  35  50 8 8 8 "
      "  65  35  50 8 8 8 "
      "  35  65  50 8 8 8 "
      "  65  65  50 8 8 8 "
      "   0   0   4 8 8 8 "
      " // target positions and widths in percentage coordinates",
    "Application:Targets int TargetColor= 0x808080 % % % "
       " // target color (color)",
    "Application:Targets string TargetTexture= % % % % "
      " // path of target texture (inputfile)",
    "Application:Targets int TestAllTargets= 0 0 0 1 "
      " // test all targets for cursor collision? "
          "0: test only the visible current target, "
          "1: test all targets "
          "(enumeration)",
  END_PARAMETER_DEFINITIONS

  BEGIN_STATE_DEFINITIONS
    "CursorPosX " CURSOR_POS_BITS " 0 0 0",
    "CursorPosY " CURSOR_POS_BITS " 0 0 0",
    "CursorPosZ " CURSOR_POS_BITS " 0 0 0",
  END_STATE_DEFINITIONS

  LANGUAGES "German",
  BEGIN_LOCALIZED_STRINGS
   "Timeout",          "Inaktiv",
   "Be prepared ...",  "Achtung ...",
  END_LOCALIZED_STRINGS

  GUI::Rect rect = { 0, 0, 1, 1 };
  mpMessage = new TextField( mWindow );
  mpMessage->SetTextColor( RGBColor::Lime )
            .SetTextHeight( 0.15 )
            .SetColor( RGBColor::NullColor )
            .SetDisplayRect( rect );
}

CursorFeedbackTask::~CursorFeedbackTask()
{
}

void
CursorFeedbackTask::OnPreflight( const SignalProperties& /*Input*/ ) const
{
  Parameter( "WindowHeight" );
  Parameter( "WindowWidth" );
  Parameter( "WindowLeft" );
  Parameter( "WindowTop" );

  const char* vectorParams[] =
  {
    "CameraPos",
    "CameraAim",
    "LightSourcePos",
    "CursorPos",
  };
  for( size_t i = 0; i < sizeof( vectorParams ) / sizeof( *vectorParams ); ++i )
    if( Parameter( vectorParams[ i ] )->NumValues() != 3 )
      bcierr << "Parameter \"" << vectorParams[ i ] << "\" must have 3 entries" << endl;

  Parameter( "WorkspaceBoundaryColor" );
  const char* colorParams[] =
  {
    "CursorColorBack",
    "CursorColorFront",
    "TargetColor",
    "LightSourceColor",
    // WorkspaceBoundaryColor may be NullColor to indicate invisibility
  };
  for( size_t i = 0; i < sizeof( colorParams ) / sizeof( *colorParams ); ++i )
    if( RGBColor( Parameter( colorParams[ i ] ) == RGBColor::NullColor ) )
      bcierr << "Invalid RGB value in " << colorParams[ i ] << endl;

  bool showTextures = ( Parameter( "RenderingQuality" ) > 0 );
  const char* texParams[] =
  {
    "CursorTexture",
    "TargetTexture",
    "WorkspaceBoundaryTexture",
  };
  for( size_t i = 0; i < sizeof( texParams ) / sizeof( *texParams ); ++i )
  {
    string filename = Parameter( texParams[ i ] );
    if( showTextures && !filename.empty() )
    {
      filename = BCIDirectory::AbsolutePath( filename );
      bool err = !ifstream( filename.c_str() ).is_open();
      if( !err )
      {
        AUX_RGBImageRec* pImg = ::auxDIBImageLoad( filename.c_str() );
        if( pImg != NULL )
        {
          ::free( pImg->data );
          ::free( pImg );
        }
        err = ( pImg == NULL );
      }
      if( err )
        bcierr << "Invalid texture file \"" << filename << "\""
               << " given in parameter " << texParams[ i ]
               << endl;
    }
  }

  if( Parameter( "NumberTargets" ) > Parameter( "Targets" )->NumRows() )
    bcierr << "The Targets parameter must contain at least NumberTargets "
           << "target definitions. "
           << "Currently, Targets contains "
           << Parameter( "Targets" )->NumRows()
           << " target definitions, and NumberTargets is "
           << Parameter( "NumberTargets" )
           << endl;

  if( MeasurementUnits::ReadAsTime( Parameter( "FeedbackDuration" ) ) <= 0 )
    bcierr << "FeedbackDuration must be greater 0" << endl;
}

void
CursorFeedbackTask::OnInitialize( const SignalProperties& /*Input*/ )
{
  mTargets.clear();
  delete mpScene;
  mpScene = new Scene( mWindow );
  mpCursor = new sphere( *mpScene );
  mpBoundary = new invertedCuboid( *mpScene );
  // The drawingOrder property allows correct z ordering in the absence of depth
  // testing.
  mpBoundary->setDrawingOrder( -2 );
  mpCursor->setDrawingOrder( -1 );

  mpScene->CameraAndLight().setQuality( Parameter( "RenderingQuality" ) );

  enum { x, y, z, dx, dy, dz };
  ParamRef CameraPos = Parameter( "CameraPos" );
  mpScene->CameraAndLight().setCamViewPoint(
    CameraPos( x ),
    CameraPos( y ),
    CameraPos( z )
  );
  ParamRef CameraAim = Parameter( "CameraAim" );
  mpScene->CameraAndLight().setCamAim(
    CameraAim( x ),
    CameraAim( y ),
    CameraAim( z )
  );
  mpScene->CameraAndLight().setCamUp( 0, 1, 0 );

  enum { flat = 0, wideAngle, narrowAngle };
  switch( int( Parameter( "CameraProjection" ) ) )
  {
    case flat:
      mpScene->CameraAndLight().setFieldOfView( 0 );
      break;
    case wideAngle:
      mpScene->CameraAndLight().setFieldOfView( 60 );
      break;
    case narrowAngle:
      mpScene->CameraAndLight().setFieldOfView( 35 );
      break;
  }
  ParamRef LightSourcePos = Parameter( "LightSourcePos" );
   mpScene->CameraAndLight().setLight(
    LightSourcePos( x ),
    LightSourcePos( y ),
    LightSourcePos( z )
  );
  RGBColor lightSourceColor = RGBColor( Parameter( "LightSourceColor" ) );
  mpScene->CameraAndLight().setLightColor(
    lightSourceColor.R()/255.,
    lightSourceColor.G()/255.,
    lightSourceColor.B()/255.
  );
  mpScene->CameraAndLight().setLightBri( 0.8 );
  mpScene->CameraAndLight().setAmbLightBri( 0.2 );

  bool showTextures = ( Parameter( "RenderingQuality" ) > 0 );

  mpBoundary->setOrigin( 50, 50, 50 );
  mpBoundary->setDimensions( 100, 100, 100 );
  mpBoundary->setFaceVisible( 0, false );
  mpBoundary->setBrightness( 1 );
  RGBColor boundaryColor = RGBColor( Parameter( "WorkspaceBoundaryColor" ) );
  mpBoundary->setColor(
    boundaryColor.R()/255.,
    boundaryColor.G()/255.,
    boundaryColor.B()/255.
  );
  mpBoundary->setVisible( boundaryColor != RGBColor::NullColor );
  string boundaryTexture;
  if( showTextures )
    boundaryTexture = Parameter( "WorkspaceBoundaryTexture" );
  if( !boundaryTexture.empty() )
    boundaryTexture = BCIDirectory::AbsolutePath( boundaryTexture );
  mpBoundary->setTexture( boundaryTexture );
  mpScene->CameraAndLight().setBoundingBox( *mpBoundary );

  mpCursor->setOrigin( 0, 0, 0 );
  mpCursor->setRadius( Parameter( "CursorWidth" ) / 2 );
  mCursorColorFront = RGBColor( Parameter( "CursorColorFront" ) );
  mCursorColorBack = RGBColor( Parameter( "CursorColorBack" ) );
  ParamRef CursorPos = Parameter( "CursorPos" );
  CursorMoveTo( CursorPos( x ), CursorPos( y ), CursorPos( z ) );
  mpCursor->setBrightness( 1 );
  string cursorTexture;
  if( showTextures )
    cursorTexture = Parameter( "CursorTexture" );
  if( !cursorTexture.empty() )
    cursorTexture = BCIDirectory::AbsolutePath( cursorTexture );
  mpCursor->setTexture( cursorTexture );
  mpCursor->setVisible( false );

  string targetTexture;
  if( showTextures )
    targetTexture = Parameter( "TargetTexture" );
  if( !targetTexture.empty() )
    targetTexture = BCIDirectory::AbsolutePath( targetTexture );
  ParamRef Targets = Parameter( "Targets" );
  for( int i = 0; i < Parameter( "NumberTargets" ); ++i )
  {
    cuboid* pTarget = new cuboid( *mpScene,
      Targets( i, x ),  Targets( i, y ),  Targets( i, z ),
      Targets( i, dx ), Targets( i, dy ), Targets( i, dz )
    );
    pTarget->setBrightness( 1 );
    pTarget->setTexture( targetTexture );
    pTarget->setVisible( false );
    mTargets.push_back( pTarget );
  }

  // Cursor speed in pixels per signal block duration:
  float feedbackDuration = MeasurementUnits::ReadAsTime( Parameter( "FeedbackDuration" ) );
  // On average, we need to cross half the workspace during a trial.
  mCursorSpeedX = 100.0 / feedbackDuration / 2;
  mCursorSpeedY = 100.0 / feedbackDuration / 2;
  mCursorSpeedZ = 100.0 / feedbackDuration / 2;
  mMaxFeedbackDuration = MeasurementUnits::ReadAsTime( Parameter( "MaxFeedbackDuration" ) );

  mWindow.SetLeft( Parameter( "WindowLeft" ) );
  mWindow.SetTop( Parameter( "WindowTop" ) );
  mWindow.SetWidth( Parameter( "WindowWidth" ) );
  mWindow.SetHeight( Parameter( "WindowHeight" ) );

  mpScene->SetBitDepth( Parameter( "WindowBitDepth" ) );
  mpScene->SetDoubleBuffering( true );
  mpScene->SetDisableVsync( true );
  GUI::Rect rect = { 0, 0, 1, 1 };
  mpScene->SetDisplayRect( rect );

  mWindow.Show();
  DisplayMessage( LocalizableString( "Timeout" ) );
  mWindow.Update();
}

void
CursorFeedbackTask::OnStartRun()
{
  ++mRunCount;
  mTrialCount = 0;
  AppLog << "Run #" << mRunCount << " started" << endl;

  DisplayMessage( LocalizableString( "Be prepared ..." ) );
  mWindow.Update();
}

void
CursorFeedbackTask::OnStopRun()
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

  DisplayMessage( LocalizableString( "Timeout" ) );
  mWindow.Update();
}

void
CursorFeedbackTask::OnTrialBegin()
{
  ++mTrialCount;
  AppLog.Screen << "Trial #" << mTrialCount
                << ", target: " << State( "TargetCode" )
                << endl;

  DisplayMessage( "" );
  RGBColor color = RGBColor( Parameter( "TargetColor" ) );
  for( int i = 0; i < int( mTargets.size() ); ++i )
  {
    mTargets[ i ]->setVisible( State( "TargetCode" ) == i + 1 );
    mTargets[ i ]->setColor( color.R()/255., color.G()/255., color.B()/255. );
  }
  mpScene->Invalidate();
  mWindow.Update();
}

void
CursorFeedbackTask::OnTrialEnd()
{
  DisplayMessage( "" );
  mpCursor->setVisible( false );
  for( size_t i = 0; i < mTargets.size(); ++i )
    mTargets[ i ]->setVisible( false );
  mpScene->Invalidate();
  mWindow.Update();
}

void
CursorFeedbackTask::OnFeedbackBegin()
{
  mCurFeedbackDuration = 0;

  enum { x, y, z };
  ParamRef CursorPos = Parameter( "CursorPos" );
  CursorMoveTo( CursorPos( x ), CursorPos( y ), CursorPos( z ) );
  mpCursor->setVisible( true );
  mpScene->Invalidate();
  mWindow.Update();
}

void
CursorFeedbackTask::OnFeedbackEnd()
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
      RGBColor color = RGBColor::Yellow;
      mpCursor->setColor( color.R()/255., color.G()/255., color.B()/255. );
      mTargets[ State( "ResultCode" ) - 1 ]->setColor( color.R()/255., color.G()/255., color.B()/255. );
      mpScene->Invalidate();
      AppLog.Screen << "-> hit" << endl;
    }
    else
    {
      AppLog.Screen << "-> miss" << endl;
    }
  }
  mWindow.Update();
}

void
CursorFeedbackTask::DoPreRun( const GenericSignal&, bool& /*doProgress*/ )
{
}

void
CursorFeedbackTask::DoPreFeedback( const GenericSignal&, bool& /*doProgress*/ )
{
}

void
CursorFeedbackTask::DoFeedback( const GenericSignal& ControlSignal, bool& doProgress )
{
  float x = mpCursor->getOriginX(),
        y = mpCursor->getOriginY(),
        z = mpCursor->getOriginZ();
  if( ControlSignal.Channels() > 0 )
    x += mCursorSpeedX * ControlSignal( 0, 0 );
  if( ControlSignal.Channels() > 1 )
    y += mCursorSpeedY * ControlSignal( 1, 0 );
  if( ControlSignal.Channels() > 2 )
    z += mCursorSpeedZ * ControlSignal( 2, 0 );

  // Restrict cursor movement to the inside of the bounding box:
  float r = mpCursor->getRadius();
  x = std::max( r, std::min( 100 - r, x ) ),
  y = std::max( r, std::min( 100 - r, y ) ),
  z = std::max( r, std::min( 100 - r, z ) );
  CursorMoveTo( x, y, z );

  if( Parameter( "TestAllTargets" ) != 0 )
  {
    for( int i = 0; State( "ResultCode" ) == 0 && i < Parameter( "NumberTargets" ); ++i )
      if( sceneObj::VolumeIntersection( *mpCursor, *mTargets[ i ] ) )
        State( "ResultCode" ) = i + 1;
  }
  else
  {
    if( sceneObj::VolumeIntersection( *mpCursor, *mTargets[ State( "TargetCode" ) - 1 ] ) )
      State( "ResultCode" ) = State( "TargetCode" );
  }
  doProgress = ( ++mCurFeedbackDuration > mMaxFeedbackDuration );
  doProgress = doProgress || ( State( "ResultCode" ) != 0 );
  mWindow.Update();
}

void
CursorFeedbackTask::DoPostFeedback( const GenericSignal&, bool& /*doProgress*/ )
{
}

void
CursorFeedbackTask::DoITI( const GenericSignal&, bool& /*doProgress*/ )
{
}

// Access to 3D objects
void
CursorFeedbackTask::CursorMoveTo( float inX, float inY, float inZ )
{
  mpCursor->setOrigin( inX, inY, inZ );
  // Adjust the cursor's color according to its z position:
  float z = inZ / 100;
  RGBColor color = z * mCursorColorFront + ( 1 - z ) * mCursorColorBack;
  mpCursor->setColor( color.R()/255., color.G()/255., color.B()/255. );
  mpScene->Invalidate();

  const float coordToState = ( 1 << cCursorPosBits - 1 ) / 100.0;
  State( "CursorPosX" ) = inX * coordToState;
  State( "CursorPosY" ) = inY * coordToState;
  State( "CursorPosZ" ) = inZ * coordToState;
}

void
CursorFeedbackTask::DisplayMessage( const string& inMessage )
{
#if 0 // Does not work reliably in conjunction with OpenGL.
  if( inMessage.empty() )
  {
    mpMessage->Hide();
  }
  else
  {
    mpMessage->SetText( string( " " ) + inMessage + " " );
    mpMessage->Show();
  }
#endif
}


