////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: griffin.milsap@gmail.com
// Description: A helper filter which acts on Gaze data from an eyetracker
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "GazeMonitorFilter.h"
#include "BCIDirectory.h"
#include "MeasurementUnits.h"
#include "DisplayWindow.h"

// Define visualization properties
#define EYE_RADIUS 0.05f
#define FIXATION_IMAGE_SIZE 0.02f
#define CORRECTION_TIME "4.0s"

#define CREATE_OBJ( obj_, type_ ) \
  for( int i_ = 0; i_ < NUM_DISPLAYS; i_++ ) \
    obj_##[i_] = new type_##( *( mpDisplays[i_] ) )

#define OBJ_METHOD( obj_, method_, ... ) \
  for( int i_ = 0; i_ < NUM_DISPLAYS; i_++ ) \
    obj_##[i_]->##method_##( __VA_ARGS__ ) \

#define POSITION_OBJ( obj_, cx_, cy_, rad_ ) \
  for( int i_ = 0; i_ < NUM_DISPLAYS; i_++ ) \
    SetDisplayRect( obj_##[i_], cx_, cy_, rad_ ) \

#define DELETE_OBJ( obj_ ) \
  for( int i_ = 0; i_ < NUM_DISPLAYS; i_++ ) { \
    delete( obj_##[i_] ); obj_##[i_] = NULL; } \

using namespace std;

RegisterFilter( GazeMonitorFilter, 3.A );

GazeMonitorFilter::GazeMonitorFilter() : 
  mEnforceFixation( false ),
  mFixationRadius( 0.0f ),
  mpFixationImage( NULL ),
  mpFixationViolationImage( NULL ),
  mpPrompt( NULL ),
  mVisualizeGaze( false ),
  mLogGazeInformation( false ),
  mFixated( true ),
  mScale( 0.0f ),
  mOffset( 0.0f ),
  mLoggingGaze( false ),
  mLoggingEyePos( false ),
  mLoggingEyeDist( false ),
  mCorrection( 0 ),
  mLostRightEye( false ),
  mLostLeftEye( false ),
  mAspectRatio( 1.0 ),
  mBlinkTime( 0 ),
  mBlinkBlocks( 0 ),
  mSaccadeTime( 0 ),
  mSaccadeBlocks( 0 ),
  mTemporalDecimation( 1 ),
  mBlockCount( 0 ),
  mVis( "APSC:1" )
{
  // Define the GazeMonitor Parameters
  BEGIN_PARAMETER_DEFINITIONS
    "Application:GazeMonitor int EnforceFixation= 0 0 0 1 "
      " // Enforce a fixation point on the screen (boolean)",
    "Application:GazeMonitor string FixationX= 0.5 0.5 % % "
      " // X coordinate of fixation center - 0.0 to 1.0 (expression)",
    "Application:GazeMonitor string FixationY= 0.5 0.5 % % "
      " // Y coordinate of fixation center - 0.0 to 1.0 (expression)",
    "Application:GazeMonitor float BlinkTime= 100ms 100ms 0 %"
      " // Maximum alloted time for invalid eyes",
    "Application:GazeMonitor float SaccadeTime= 100ms 100ms 0 %"
      " // Maximum alloted time for violating the fixation",
    "Application:GazeMonitor float FixationRadius= 0.1 0.1 0.0 1.0 "
      " // Allowable distance from fixation center",
    "Application:GazeMonitor string FixationImage= % % % % "
      " // Image to render at fixation (inputfile)",
    "Application:GazeMonitor string FixationViolationImage= % % % % "
      " // Image to render at fixation when violated (inputfile)",
    "Application:GazeMonitor string FixationViolationSound= % % % % "
      " // Sound to play when fixation violated (inputfile)",
    "Application:GazeMonitor int LogGazeInformation= 0 0 0 1 "
      " // Add Eyetracker information to applog (boolean)",
  END_PARAMETER_DEFINITIONS

  // Define the GazeMonitor States
  BEGIN_STATE_DEFINITIONS
    "FixationViolated 1 0 0 0",
    "Correction       1 0 0 0", // Set this state true to enable user correction
  END_STATE_DEFINITIONS

  mpZone[APP] = NULL; mpZone[VIS] = NULL;
  mpCursor[APP] = NULL; mpCursor[VIS] = NULL;
  mpLeftEye[APP] = NULL; mpLeftEye[VIS] = NULL;
  mpRightEye[APP] = NULL; mpRightEye[VIS] = NULL;

  // FIXME: When Application DisplayWindow is available, put it here!
  GUI::DisplayWindow* appwindow = new GUI::DisplayWindow();
  appwindow->SetLeft( -1280 ); appwindow->SetTop( 0 );
  appwindow->SetWidth( 1280 ); appwindow->SetHeight( 1024 );
  appwindow->SetTitle( "Application Overlay" ); 
  appwindow->SetColor( RGBColor::Black );
  appwindow->Show(); appwindow->Update();
  mpDisplays[APP] = appwindow;

  mpDisplays[VIS] = new GUI::GraphDisplay();
}

GazeMonitorFilter::~GazeMonitorFilter()
{
  DeleteStimuli();
  DELETE_OBJ( mpDisplays );
}

void 
GazeMonitorFilter::Preflight( const SignalProperties &Input, SignalProperties &Output ) const
{
  bool loggingGaze = false, loggingEyePos = false, loggingEyeDist = false;

  // See what we're logging and check states accordingly
  if( ( int )OptionalParameter( "LogGazeData", 0 ) )
  {
    loggingGaze = true;
    State( "EyetrackerLeftEyeGazeX" );
    State( "EyetrackerLeftEyeGazeY" );
    State( "EyetrackerRightEyeGazeX" );
    State( "EyetrackerRightEyeGazeY" );
    Parameter( "GazeScale" );
    Parameter( "GazeOffset" );
  }
  if( ( int )OptionalParameter( "LogEyePos", 0 ) )
  {
    loggingEyePos = true;
    State( "EyetrackerLeftEyePosX" );
    State( "EyetrackerLeftEyePosY" );
    State( "EyetrackerRightEyePosX" );
    State( "EyetrackerRightEyePosY" );
  }
  if( ( int )OptionalParameter( "LogEyeDist", 0 ) )
  {
    loggingEyeDist = true;
    State( "EyetrackerLeftEyeDist" );
    State( "EyetrackerRightEyeDist" );
  }

  // We can require eyetracker validity
  State( "EyetrackerLeftEyeValidity" );
  State( "EyetrackerRightEyeValidity" );

  bool enforceFixation = ( int )Parameter( "EnforceFixation" );
  bool vizGaze = ( int )Parameter( "VisualizeGazeMonitorFilter" );
  if( vizGaze )
  {
    Parameter( "WindowWidth" );
    Parameter( "WindowHeight" );
    Parameter( "AppWindowSpatialDecimation" );
    Parameter( "AppWindowTemporalDecimation" );
  }

  // Do some preflight error checking
  if( enforceFixation )
  {
    if( !loggingGaze )
      bcierr << "In order to use GazeMonitor, you must at least be logging gaze data into states." << endl;

    State( "FixationViolated" );
    GenericSignal preflightInput( Input );
    if( string( Parameter( "FixationX" ) ) == "" )
      bcierr << "Requested fixation enforcement but specified no FixationX coordinate." << endl;
    else Expression( Parameter( "FixationX" ) ).Evaluate( &preflightInput );
   
    if( string( Parameter( "FixationY" ) ) == "" )
      bcierr << "Requested fixation enforcement but specified no FixationY coordinate." << endl;
    else Expression( Parameter( "FixationY" ) ).Evaluate( &preflightInput );

    if( Parameter( "FixationRadius" ) < 0.0001 )
      bcierr << "Requested fixation enforcement but specified no FixationRadius." << endl;

    // Check if requested images exist and try to load them
    GUI::GraphDisplay preflightDisplay;
    ImageStimulus pimg( preflightDisplay );
    if( string( Parameter( "FixationImage" ) ) != "" )
      pimg.SetFile( string( Parameter( "FixationImage" ) ) );
    if( string( Parameter( "FixationViolationImage" ) ) != "" )
      pimg.SetFile( string( Parameter( "FixationViolationImage" ) ) );

    // Check if the requested sound exists and try to load it
    if( string( Parameter( "FixationViolationSound" ) ) != "" )
    {
      string filename = string( Parameter( "FixationViolationSound" ) );
      WavePlayer wp;
      InitSound( filename, wp );
    }
    Parameter( "BlinkTime" );
    Parameter( "SaccadeTime" );
  }
  Parameter( "LogGazeInformation" );
}

void 
GazeMonitorFilter::Initialize( const SignalProperties &Input, const SignalProperties &Output )
{
  DeleteStimuli();
  
  mLogGazeInformation = ( int )Parameter( "LogGazeInformation" );
  //if( mLogGazeInformation ) AppLog << "GazeMonitorFilter Initialized." << endl;
  mLostLeftEye = false; mLostRightEye = false;

  mLoggingGaze = ( int )OptionalParameter( "LogGazeData", 0 ); 
  mLoggingEyePos = ( int )OptionalParameter( "LogEyePos", 0 ); 
  mLoggingEyeDist = ( int )OptionalParameter( "LogEyeDist", 0 );
  mEnforceFixation = ( int )Parameter( "EnforceFixation" );
  mVisualizeGaze = ( int )Parameter( "VisualizeGazeMonitorFilter" );

  // If we're logging gaze at all, we need extra information to process it.
  if( mLoggingGaze )
  {
    mScale = ( float )Parameter( "GazeScale" );
    mOffset = ( float )Parameter( "GazeOffset" );
  }

  if( mVisualizeGaze )
  {
    int visWidth = Parameter( "WindowWidth" );
    int visHeight = Parameter( "WindowHeight" );
    mAspectRatio = ( float )visWidth / ( float )visHeight;
    visWidth /= Parameter( "AppWindowSpatialDecimation" );
    visHeight /= Parameter( "AppWindowSpatialDecimation" );
    mTemporalDecimation = Parameter( "AppWindowTemporalDecimation" );
    GUI::DrawContext dc = mpDisplays[VIS]->Context();
    GUI::Rect r = { 0, 0, visWidth, visHeight };
    dc.rect = r;
    mpDisplays[VIS]->SetContext( dc );
    mpDisplays[VIS]->SetColor( RGBColor::NullColor );
  }

  if( mEnforceFixation )
  {
    GenericSignal initInput( Input );
    mFixationX = Expression( string( Parameter( "FixationX" ) ) );
    mFixationY = Expression( string( Parameter( "FixationY" ) ) );
    mFixationRadius = ( float ) Parameter( "FixationRadius" );

    // Calculate current requested center of fixation
    float cx = mFixationX.Evaluate( &initInput );
    float cy = mFixationY.Evaluate( &initInput );

    // Set up the fixation image
    if( string( Parameter( "FixationImage" ) ) != "" )
    {
      mpFixationImage = new ImageStimulus( *( mpDisplays[APP] ) );
      mpFixationImage->SetFile( string( Parameter( "FixationImage" ) ) );
      mpFixationImage->SetRenderingMode( GUI::RenderingMode::Transparent );
      mpFixationImage->SetAspectRatioMode( GUI::AspectRatioModes::AdjustWidth );
      SetDisplayRect( mpFixationImage, cx, cy, FIXATION_IMAGE_SIZE );
      mpFixationImage->SetPresentationMode( VisualStimulus::ShowHide );
      mpFixationImage->Present();
    }

    // Set up the fixation violation image
    if( string( Parameter( "FixationViolationImage" ) ) != "" )
    {
      mpFixationViolationImage = new ImageStimulus( *( mpDisplays[APP] ) );
      mpFixationViolationImage->SetFile( string( Parameter( "FixationViolationImage" ) ) );
      mpFixationViolationImage->SetRenderingMode( GUI::RenderingMode::Transparent );
      mpFixationViolationImage->SetAspectRatioMode( GUI::AspectRatioModes::AdjustWidth );
      SetDisplayRect( mpFixationViolationImage, cx, cy, FIXATION_IMAGE_SIZE );
      mpFixationViolationImage->SetPresentationMode( VisualStimulus::ShowHide );
      mpFixationViolationImage->Conceal();
    }

    // Set up the violation sound
    string filename = string( Parameter( "FixationViolationSound" ) );
    if( filename != "" )
	  InitSound( filename, mViolationSound );

    // Set up fixation zone visualization
    CREATE_OBJ( mpZone, EllipticShape );
    OBJ_METHOD( mpZone, SetColor, RGBColor::Gray );
    OBJ_METHOD( mpZone, SetFillColor, RGBColor::NullColor );
    OBJ_METHOD( mpZone, SetAspectRatioMode, GUI::AspectRatioModes::AdjustWidth );
    OBJ_METHOD( mpZone, SetLineWidth, 4.0f );
    POSITION_OBJ( mpZone, cx, cy, mFixationRadius );
    OBJ_METHOD( mpZone, Show );

    // Create a prompt for user correction
    mpPrompt = new TextField( *( mpDisplays[APP] ) );
    mpPrompt->SetTextColor( RGBColor::White );
    GUI::Rect textRect = { 0.45f, 0.50f, 0.55f, 0.60f };
    mpPrompt->SetDisplayRect( textRect );
    mpPrompt->SetTextHeight( 0.1f );
    mpPrompt->Hide();

    mBlinkTime = Parameter( "BlinkTime" ).InSampleBlocks();
    mBlinkBlocks = 0;
    mSaccadeTime = Parameter( "SaccadeTime" ).InSampleBlocks();
    mSaccadeBlocks = 0;

    //if( mLogGazeInformation ) AppLog << "Enforcing Fixation." << endl;
    mFixated = true;
  }

  if( mLoggingGaze )
  {
    CREATE_OBJ( mpCursor, EllipticShape );
    OBJ_METHOD( mpCursor, SetZOrder, 0 );
    OBJ_METHOD( mpCursor, SetColor, RGBColor::Black );
    OBJ_METHOD( mpCursor, SetAspectRatioMode, GUI::AspectRatioModes::AdjustWidth );
    OBJ_METHOD( mpCursor, Hide );
  }     
  if( mLoggingEyePos )
  {
    CREATE_OBJ( mpRightEye, EllipticShape );
    CREATE_OBJ( mpLeftEye, EllipticShape );
    OBJ_METHOD( mpRightEye, SetZOrder, 5 ); OBJ_METHOD( mpLeftEye, SetZOrder, 5 );
    OBJ_METHOD( mpRightEye, SetColor, RGBColor::Black ); OBJ_METHOD( mpLeftEye, SetColor, RGBColor::Black );
    OBJ_METHOD( mpRightEye, SetFillColor, RGBColor::DkGray ); OBJ_METHOD( mpLeftEye, SetFillColor, RGBColor::DkGray );
    OBJ_METHOD( mpRightEye, SetAspectRatioMode, GUI::AspectRatioModes::AdjustWidth );
    OBJ_METHOD( mpLeftEye, SetAspectRatioMode, GUI::AspectRatioModes::AdjustWidth );
    OBJ_METHOD( mpRightEye, Hide ); OBJ_METHOD( mpLeftEye, Hide );
  }
}

void
GazeMonitorFilter::StartRun() 
{
  // Ensure We're not in "Correction" state
  State( "Correction" ) = 0;
  mCorrection = 0;
  mpPrompt->Hide();
  OBJ_METHOD( mpZone, SetColor, RGBColor::Gray );

  if( mVisualizeGaze )
  {
    mpDisplays[VIS]->Update();
    mpDisplays[VIS]->Paint();
    mVis.SendReferenceFrame( mpDisplays[VIS]->BitmapData() );
    mBlockCount = mTemporalDecimation - 1;
  }
}

void 
GazeMonitorFilter::Process( const GenericSignal &Input, GenericSignal &Output )
{
  // Determine gaze location
  float gx = 0.0f, gy = 0.0f;
  if( mLoggingGaze )
  {
    gx = ( float )State( "EyetrackerLeftEyeGazeX" ) + ( float )State( "EyetrackerRightEyeGazeX" );
    gy = ( float )State( "EyetrackerLeftEyeGazeY" ) + ( float )State( "EyetrackerRightEyeGazeY" );
    gx /= ( 2.0f * 65535.0f ); gx -= mOffset; gx /= mScale;
    gy /= ( 2.0f * 65535.0f ); gy -= mOffset; gy /= mScale;
  }

  // Determine eye position
  float plx = 0.0f, ply = 0.0f;
  float prx = 0.0f, pry = 0.0f;
  if( mLoggingEyePos )
  {
    plx = 1.0f - ( ( float )State( "EyetrackerLeftEyePosX" ) / 65535.0f );
    ply = ( float )State( "EyetrackerLeftEyePosY" ) / 65535.0f;
    prx = 1.0f - ( ( float )State( "EyetrackerRightEyePosX" ) / 65535.0f );
    pry = ( float )State( "EyetrackerRightEyePosY" ) / 65535.0f;
  }

  // Determine Eye Distance
  float eyedist = 256.0f;
  if( mLoggingEyeDist )
    eyedist = ( ( float )State( "EyetrackerLeftEyeDist" ) 
              + ( float )State( "EyetrackerRightEyeDist" ) ) / 2.0f;

  // Determine (and log) Validity
  bool leftEyeValid = ( int )State( "EyetrackerLeftEyeValidity" ) < 2;
  bool rightEyeValid = ( int )State( "EyetrackerRightEyeValidity" ) < 2;
  if( mLostLeftEye || mLostRightEye )
  {
    if( leftEyeValid && rightEyeValid )
    {
      //if( mLogGazeInformation ) AppLog << "Both Eyes Tracking Correctly." << endl;
      mLostLeftEye = false;
      mLostRightEye = false;
      mBlinkBlocks = 0;
    }
    else
      mBlinkBlocks++;
  }

  if( !leftEyeValid && !mLostLeftEye )
  {
    //if( mLogGazeInformation ) AppLog << "Lost Left Eye!" << endl;
    mLostLeftEye = true;
  }

  if( !rightEyeValid && !mLostRightEye )
  {
    //if( mLogGazeInformation ) AppLog << "Lost Right Eye!" << endl;
    mLostRightEye = true;
  }
  bool eyesInvalid = ( mBlinkBlocks > mBlinkTime );

  // Update position of visualizations
  if( mLoggingGaze )
  {
    POSITION_OBJ( mpCursor, gx, gy, ( ( eyedist - 400 ) / 400 ) * 0.06 );
    OBJ_METHOD( mpCursor, SetFillColor, RGBColor( ( int )eyedist % 255, 255 - ( ( int )eyedist % 255 ), 50 ) );
    OBJ_METHOD( mpCursor, Show );
  }

  if( mLoggingEyePos )
  {
    POSITION_OBJ( mpLeftEye, plx, ply, EYE_RADIUS );
    if( leftEyeValid ) { OBJ_METHOD( mpLeftEye, Show ); }
    else { OBJ_METHOD( mpLeftEye, Hide ); }
    POSITION_OBJ( mpRightEye, prx, pry, EYE_RADIUS );
    if( rightEyeValid ) { OBJ_METHOD( mpRightEye, Show ); } 
    else { OBJ_METHOD( mpRightEye, Hide ); }
  }

  if( mEnforceFixation )
  {
    // Determine fixation center
    float fx = ( float )mFixationX.Evaluate( &Input );
    float fy = ( float )mFixationY.Evaluate( &Input );

    // Move visual stimuli to fixation
    if( mpFixationImage ) 
      SetDisplayRect( mpFixationImage, fx, fy, FIXATION_IMAGE_SIZE );
    if( mpFixationViolationImage ) 
      SetDisplayRect( mpFixationViolationImage, fx, fy, FIXATION_IMAGE_SIZE );
    POSITION_OBJ( mpZone, fx, fy, mFixationRadius );

    // Calculate distance of gaze from fixation center
    float dist = pow( ( gx - fx ) * mAspectRatio, 2.0f ) + pow( gy - fy, 2.0f );
    dist = sqrt( dist );
    bool lookingAway = false;
    if( dist > mFixationRadius )
    {
      // Subject is looking outside the fixation radius.  Is it a momentary saccade?
      if( mSaccadeBlocks <= mSaccadeTime )
        mSaccadeBlocks++;
      else // This is more than a blink
        lookingAway = true;
    }
    else
      mSaccadeBlocks = 0;

    // Fixation Violation Logic
    if( mFixated && ( eyesInvalid || lookingAway ) )
      ViolatedFixation();
    else if( !mFixated && ( !eyesInvalid && !lookingAway ) )
      AcquiredFixation();

    // If we want the subject to correct their eyetracking behavior, show them eyetracker output
    if( ( int )State( "Correction" ) )
    {
      // Give the subject a prompt
      string prompt = "";
      int correctionTime = MeasurementUnits::TimeInSampleBlocks( CORRECTION_TIME );
      if( mCorrection >= ( ( correctionTime * 3 ) / 4 ) )
        prompt = "Hold it! 1...";
      else if( mCorrection >= ( ( correctionTime * 2 ) / 4 ) )
        prompt = "Hold it! 2...";
      else if( mCorrection >= ( correctionTime / 4 ) )
        prompt = "Hold it! 3...";
      else if( mCorrection != 0 )
        prompt = "OK!";
      else
        prompt = "Fixate";
      SetDisplayRect( mpPrompt, gx, gy + 0.05f, 0.2f );
      mpPrompt->Show();

      // Determine if user has corrected fixation
      if( mFixated && eyedist >= 550 && eyedist < 600 )
      {
        mCorrection++;
        OBJ_METHOD( mpZone, SetColor, RGBColor::Green );
        if( mCorrection >= correctionTime )
        {
          //Disable drawing to the subject's screen
          State( "Correction" ) = 0;
          mCorrection = 0;
          mpPrompt->Hide();
          OBJ_METHOD( mpZone, SetColor, RGBColor::Gray );
        }
      } else if( mFixated && eyedist < 550 ) {
        prompt = "Move Away.";
        mCorrection = 0;
      } else if( mFixated && eyedist >= 600 ) {
        prompt = "Move Closer.";
        mCorrection = 0;
      } else {
        mCorrection = 0;
        OBJ_METHOD( mpZone, SetColor, RGBColor::Gray );
      }
      mpPrompt->SetText( prompt );
    }
  }

  // Using correction without fixation enforcement is a no-no.
  if( ( int )State( "Correction" ) && !mEnforceFixation )
  {
    bciout << "Attempting to use eyetracker correction without fixation enforcement." << endl;
    State( "Correction" ) = 0;
  }

  // Draw preview frame
  if( mVisualizeGaze && ( ++mBlockCount %= mTemporalDecimation ) == 0 )
  {
    mpDisplays[VIS]->Paint();
    BitmapImage b = mpDisplays[VIS]->BitmapData();
    mVis.SendDifferenceFrame( b );
  }

  Output = Input;
}

void
GazeMonitorFilter::InitSound( const string& inFilename, WavePlayer& ioPlayer ) const
{
    if( ioPlayer.ErrorState() != WavePlayer::noError )
      bcierr << "There was an issue creating a waveplayer object." << endl;
	ioPlayer.SetFile( BCIDirectory::AbsolutePath( inFilename ) );
	if( ioPlayer.ErrorState() != WavePlayer::noError )
      bcierr << "Could not open file: " << BCIDirectory::AbsolutePath( inFilename ) << endl;
    ioPlayer.SetVolume( 1.0f );
}

void
GazeMonitorFilter::SetDisplayRect( GUI::GraphObject *obj, float cx, float cy, float rad )
{
  GUI::Rect rect = { cx - ( rad / mAspectRatio ), cy - rad, 
                     cx + ( rad / mAspectRatio ), cy + rad };
  obj->SetDisplayRect( rect );
}

void
GazeMonitorFilter::DeleteStimuli()
{
  // Perform a bit of cleanup
  delete mpFixationImage; mpFixationImage = NULL;
  delete mpFixationViolationImage; mpFixationViolationImage = NULL;
  delete mpPrompt; mpPrompt = NULL;
  DELETE_OBJ( mpRightEye );
  DELETE_OBJ( mpLeftEye );
  DELETE_OBJ( mpCursor );
  DELETE_OBJ( mpZone );
}

void
GazeMonitorFilter::ViolatedFixation()
{
  //if( mLogGazeInformation ) AppLog << "Gaze Left Fixation." << endl;
  mViolationSound.Play();
  State( "FixationViolated" ) = true;
  if( mpFixationImage ) mpFixationImage->Conceal();
  if( mpFixationViolationImage ) mpFixationViolationImage->Present();
  mFixated = false;
}

void
GazeMonitorFilter::AcquiredFixation()
{
  //if( mLogGazeInformation ) AppLog << "Gaze Fixation Acquired." << endl;
  State( "FixationViolated" ) = false;
  if( mpFixationImage ) mpFixationImage->Present();
  if( mpFixationViolationImage ) mpFixationViolationImage->Conceal();
  mFixated = true;
}