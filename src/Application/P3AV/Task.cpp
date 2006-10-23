#include "PCHIncludes.h"
#pragma hdrstop

#include "Task.h"
#include "UsrEnv.h"
#include "UsrEnvDispatcher.h"
#include "UsrEnvAlgorithmP3AV.h"
#include "WavePlayer.h"
#include "UBCITime.h"
#include "UGenericVisualization.h"
#include "Localization.h"

#include <string>

using namespace std;

RegisterFilter( TTask, 3 );

// **************************************************************************
// Function:   TASK
// Purpose:    This is the constructor for the TASK class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
TTask::TTask()
: mApplicationPath( ExtractFilePath(Application->ExeName).c_str() ),
  mTaskLogVis( SOURCEID::TASKLOG ),
  m_pUsrEnvDispatcher( new UsrEnvDispatcher ),
  m_pUsrEnv( new UsrEnv(AnsiString("P3AVTask"), new UsrEnvAlgorithmP3AV) )
{
  if( mApplicationPath.empty()
      || mApplicationPath[ mApplicationPath.length() - 1 ] != '\\' )
    mApplicationPath += '\\';

  // window parameters
  BEGIN_PARAMETER_DEFINITIONS
    "P3AV_Window int WinXpos= 5 0 0 5000 // "
      "User Window X location",
    "P3AV_Window int WinYpos= 5 0 0 5000 // "
      "User Window Y location",
    "P3AV_Window int WinWidth= 512 512 0 2000 // "
      "User Window Width",
    "P3AV_Window int WinHeight= 512 512 0 2000 // "
      "User Window Height",
    "P3AV_Window string WinBackgroundColor= 0x00585858 0x00505050 0x00000000 0x00000000 // "
      "Background Color (color)",
  END_PARAMETER_DEFINITIONS

  // Stimulus parameters
  BEGIN_PARAMETER_DEFINITIONS
    "P3AV_Stimulus int StimulusWidth= 5 0 0 100 // "
      "StimulusWidth in percent of screen width",
    "P3AV_Stimulus int CaptionHeight= 10 0 0 100 // "
      "Height of stimulus caption text in percent of screen height",
    "P3AV_Stimulus string CaptionColor= 0x00FFFFFF 0x00FFFFFF 0x00000000 0x00000000 // "
      "Color of stimulus caption text (color)",
    "P3AV_Stimulus string BackgroundColor= 0x00FFFF00 0x00FFFF00 0x00000000 0x00000000 // "
      "Color of stimulus caption text (color)",
    "P3AV_Stimulus float AudioVolume= 1 1 0 1 // "
      "Volume for audio playback",
    "P3AV_Stimulus int OnTime= 10 10 0 5000 // "
      "Stimulus presentation time (in units of SampleBlocks)",
    "P3AV_Stimulus int OffTime= 3 10 0 5000 // "
      "Inter-stimulus period (in units of SampleBlocks)",
    "P3AV_Stimulus int PreSequenceTime= 50 30 0 5000 // "
      "Period preceding the presentation of the stimulus sequence (in units of SampleBlocks)",
    "P3AV_Stimulus int PostSequenceTime= 50 30 0 5000 // "
      "Period following the presentation of the stimulus sequence (in units of SampleBlocks)",
    "P3AV_Stimulus int MinInterTime= 0 0 0 0 // "
      "Minimum time that will be randomly added to the inter-stimulus interval in units of SampleBlocks",
    "P3AV_Stimulus int MaxInterTime= 3 3 0 0 // "
      "Maximum time that will be randomly added to the inter-stimulus interval in units of SampleBlocks",
    "P3AV_Stimulus int AudioSwitch= 1 1 0 1 // "
      "Present audio files (boolean)",
    "P3AV_Stimulus int VideoSwitch= 1 1 0 1 // "
      "Present icon files (boolean)",
    "P3AV_Stimulus int CaptionSwitch= 1 1 0 1 // "
      "Present captions (boolean)",
  END_PARAMETER_DEFINITIONS

  // stimuli sequence related parameters
  BEGIN_PARAMETER_DEFINITIONS
    "P3AV_Stimuli matrix Matrix= "
     "{ caption icon audio } " // row labels
     "{ stimulus1 stimulus2 stimulus3 stimulus4 stimulus5 stimulus6 } " // column labels
     " One Two Three Four Five Six "
     "images\\1.bmp images\\2.bmp images\\3.bmp images\\4.bmp images\\5.bmp images\\6.bmp "
     "sounds\\1.wav sounds\\2.wav sounds\\3.wav sounds\\4.wav sounds\\5.wav sounds\\6.wav  "
     " // captions and icons to be displayed, sounds to be played for different stimuli",

    "P3AV_Stimuli matrix FocusOn= "
     "{ caption icon audio } " // row labels
     "{ focuson } " // column labels
     "Please%20focus%20on "     // caption
     "images\\focuson.bmp " // video
     "sounds\\uh-uh.wav  "  // audio
     " // initial announcement what to focus on",

    "P3AV_Stimuli matrix Result= "
     "{ caption icon audio } " // row labels
     "{ result } " // column labels
     "The%20result%20was "     // caption
     "images\\result.bmp " // video
     "sounds\\uh-uh.wav "  // audio
     " // final result announcement ",

    "P3AV_Stimuli intlist Sequence= 4 1 3 4 2 1 1 1000 // "
      "Sequence in which stimuli are presented (deterministic mode)/ Stimulus frequencies for each stimulus (random mode)",

    "P3AV_Stimuli int SequenceType= 0 0 0 1 // "
      "Sequence of stimuli is 0 deterministic, 1 random (enumeration)",

    "P3AV_Stimuli int NumberOfSeq= 3 1 0 1000 // "
      "How many times the sequence will be played",

    "P3AV_Stimuli int InterpretMode= 0 0 0 2 // "
      "Classification of results: 0 none, 1 free mode, 2 copy mode (enumeration)",

    "P3AV_Stimuli int DisplayResults= 1 1 0 1 // "
      "Display results of copy/free spelling: 0=no, 1=yes (boolean)",

    "P3AV_Stimuli intlist ToBeCopied= 3 1 2 3 1 1 1000 // "
      "Sequence in which stimuli need to be copied (only used in copy mode)",

    "P3AV_Stimuli string UserComment= Add your comment here // "
      "User comments for a specific run",
  END_PARAMETER_DEFINITIONS

  LANGUAGES "German",
  BEGIN_LOCALIZED_STRINGS
   "TIME OUT !!!",
           "Zeit abgelaufen!",
   "Waiting to start ...",
           "Warte ...",
  END_LOCALIZED_STRINGS

  BEGIN_STATE_DEFINITIONS
    "SelectedStimulus 8 0 0 0",
    "StimulusTime 16 17528 0 0",
    "PhaseInSequence 2 0 0 0",
    "StimulusCode 8 0 0 0",
    "StimulusType 1 0 0 0",
    "Flashing 1 0 0 0",
    "PressedKey 8 0 0 0",
  END_STATE_DEFINITIONS
}

//-----------------------------------------------------------------------------


// **************************************************************************
// Function:   ~TASK
// Purpose:    This is the destructor for the TASK class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
TTask::~TTask()
{
  delete m_pUsrEnv;
  delete m_pUsrEnvDispatcher;
}


void TTask::Preflight(const SignalProperties& inputProperties,
                            SignalProperties& outputProperties ) const
{
  // check all the parameters' presence
  // This will be done from the framework some day.
  const char* paramNames[] =
  {
    "WinXpos",
    "WinYpos",
    "WinWidth",
    "WinHeight",
    "WinBackgroundColor",
    "StimulusWidth",
    "CaptionHeight",
    "CaptionColor",
    "AudioVolume",
    "OnTime",
    "OffTime",
    "PreSequenceTime",
    "PostSequenceTime",
    "MinInterTime",
    "MaxInterTime",
    "AudioSwitch",
    "VideoSwitch",
    "CaptionSwitch",
    "Matrix",
    "FocusOn",
    "Result",
    "Sequence",
    "SequenceType",
    "NumberOfSeq",
    "InterpretMode",
    "ToBeCopied",
    "UserComment",
    "SampleBlockSize",
  };
  for( int i = 0; i < sizeof( paramNames ) / sizeof( *paramNames ); ++i )
    Parameter( paramNames[ i ] ); // This results in an error message if the
                                  // parameter is missing.
  if( Parameter( "OnTime" ) == 0 )
    bcierr << "OnTime parameter can not be 0." << endl;

  if( Parameter( "InterpretMode" ) < 0 || Parameter( "InterpretMode" ) > 2 )
    bcierr << "InterpretMode parameter can be only "
              "0(none mode), 1(free mode), or 2(copy mode)" << endl;

  if( Parameter( "AudioSwitch" ) < 0 || Parameter( "AudioSwitch" ) > 1 )
    bcierr << "AudioSwitch parameter can be only 0(off), 1(on)" << endl;

  if( Parameter( "VideoSwitch" ) < 0 || Parameter( "VideoSwitch" ) > 1 )
    bcierr << "VideoSwitch parameter can be only 0(off), 1(on)" << endl;

  if( Parameter( "CaptionSwitch" ) < 0 || Parameter( "CaptionSwitch" ) > 1 )
    bcierr << "CaptionSwitch parameter can be only 0(off), 1(on)" << endl;

  if( Parameter( "MinInterTime" ) > Parameter( "MaxInterTime" ) )
    bcierr << "MinInterTime parameter can not be larger than MaxInterTime parameter" << endl;

  // The PostSequenceTime needs to be long enough so that the last stimulus classification
  // result makes it to the application. The info about when the classification is made
  // is defined in a parameter in the P3SigProc, and thus within the domain of a
  // different module. Since we are not supposed to create module interdependencies,
  // we here check it against some arbitrary time, i.e., 1 second.
  int blockspersecond=(int)(Parameter("SamplingRate")/Parameter("SampleBlockSize"));
  if( Parameter( "PostSequenceTime" ) < blockspersecond )
    bcierr << "PostSequenceTime parameters must be"
              " at least 1 second long, i.e., " << blockspersecond << " blocks." << endl;

  // check stimuli matrix , FocusOn matrix, Result matrix
  ErrorReadingMatrix("Matrix");
  ErrorReadingMatrix("FocusOn");
  ErrorReadingMatrix("Result");

  // now check the states
  const char* stateNames[] =
  {
    "SelectedStimulus",
    "StimulusTime",
    "PhaseInSequence",
    "StimulusCode",
    "StimulusType",
    "Flashing",
    "PressedKey",
    // "StimulusCodeRes", // This state is accessed from UsrEnvDispatcher and
                          // appears to be optional there.
    "Running",
  };
  for( int i = 0; i < sizeof( stateNames ) / sizeof( *stateNames ); ++i )
    State( stateNames[ i ] );  // This results in an error message if the
                               // state is missing.

  // check whether sound card is present
  if( ::waveOutGetNumDevs() == 0 )
    bcierr << "Sound card is missing." << endl;

  PreflightCondition( inputProperties >= SignalProperties( 1, 1, SignalType::int16 ) );
  outputProperties = inputProperties;
} // Preflight


bool TTask::ErrorReadingMatrix( const string& sMatrixName ) const
{
  bool result = false;
  for( size_t i = 0; i < Parameter( sMatrixName )->GetNumValuesDimension2(); ++i )
  {
    // check sound files
    if( Parameter( "AudioSwitch" ) == 1 )
    {
      string soundFileName = ( const char* )Parameter( sMatrixName.c_str(), "audio", i );
      if( soundFileName != "" )
      {
        soundFileName = mApplicationPath + soundFileName;
        result = result || ErrorLoadingAudioFile( soundFileName );
      }
    }
    // check icon files
    if( Parameter( "VideoSwitch" ) == 1 )
    {
      string iconFileName = ( const char* )Parameter( sMatrixName.c_str(), "icon", i );
      if( iconFileName != "" )
      {
        iconFileName = mApplicationPath + iconFileName;
        result = result || ErrorLoadingVideoFile( iconFileName );
      }
    }
  }
  return result;
}


bool TTask::ErrorLoadingAudioFile( const string& sAudioFile ) const
{
  TWavePlayer testPlayer;
  TWavePlayer::Error err;
  if (Parameter( "AudioSwitch" ) == 1)
    err = testPlayer.AttachFile(sAudioFile.c_str());
  if( err == TWavePlayer::fileOpeningError )
  {
    bcierr << "Could not open \"" << sAudioFile << "\" as a sound file" << endl;
    return true;
  }
  else if( err != TWavePlayer::noError )
  {
    bcierr << "Some general error prevents wave audio playback" << endl;
    return true;
  }
  return false;
} // ErrorLoadingAudioFile


bool TTask::ErrorLoadingVideoFile( const string& sVideoFile ) const
{
  string errorText = "";
  TImage * pIcon = new TImage(static_cast<TComponent*>(NULL));
  try
  {
    if (Parameter( "VideoSwitch" ) == 1)
      pIcon->Picture->LoadFromFile(sVideoFile.c_str());
  }
  catch(EInOutError & )
  {
    errorText = "Input/Output error";
  }
  catch(EInvalidGraphic & )
  {
    errorText = "Invalid Graphic error";
  }
  catch (EFOpenError &)
  {
    errorText = "Opening error";
  }
  catch(Exception &)
  {
    errorText = "General error";
  }
  delete pIcon;
  if( !errorText.empty() )
    bcierr << errorText << " for \"" << sVideoFile << "\"" << endl;
  return !errorText.empty();
} // ErrorLoadingVideoFile


// **************************************************************************
// Function:   Initialize
// Purpose:    Initializes the task, e.g., resets the user env dispatcher, etc.
// Parameters:
// Returns:    N/A
// **************************************************************************
void TTask::Initialize()
{
  mTaskLogVis.Send( CFGID::WINDOWTITLE, "User Task Log" );

  // initialize user environment
  if (m_pUsrEnv != NULL)
  {
    int iWinXpos(5), iWinYpos(5), iWinWidth(512), iWinHeight(512);
    TColor backgroundColor = clDkGray;
    backgroundColor = (TColor)strtol( Parameter( "WinBackgroundColor" ), NULL, 16 );
    iWinXpos  = Parameter( "WinXpos" );
    iWinYpos  = Parameter( "WinYpos" );
    iWinWidth = Parameter( "WinWidth" );
    iWinHeight= Parameter( "WinHeight" );
    m_pUsrEnv->Initialize(this, Application, iWinYpos, iWinXpos, iWinWidth, iWinHeight, backgroundColor);
  }

  // initialize user environment dispatcher
  if (m_pUsrEnvDispatcher != NULL)
  {
    bool shouldinterpret=false;
    if (Parameter("InterpretMode") != 0) shouldinterpret=true;
    m_pUsrEnvDispatcher->Initialize(m_pUsrEnv, shouldinterpret);
  }
}


// **************************************************************************
// Function:   GetPressedKey
// Purpose:    Determines the code of the pressed key
//             This only works if the window has the focus and should be replaced
//             by a Windows hook function using SetWindowsHookEx
// Parameters: N/A
// Returns:    0 if no key is pressed or the code of the pressed key otherwise
// **************************************************************************
char TTask::GetPressedKey()
{
BYTE KeyState[256];

 GetKeyboardState((PBYTE)KeyState);

 char pressedkey=0;
 for (int i=0; i<256; i++)
  if (KeyState[i] & 128)
     {
     pressedkey=i;
     break;
     }

 return(pressedkey);
}


// **************************************************************************
// Function:   Process
// Purpose:    Processes the control signal sent by the frame work
// Parameters: signals - pointer to the vector of controlsignals (1st element = up/down, 2nd element = left/right)
// Returns:    N/A
// **************************************************************************
void TTask::Process( const GenericSignal* Input,
                           GenericSignal* Output )
{
  // creates a new state of the user environment
  if (m_pUsrEnvDispatcher != NULL)
    m_pUsrEnvDispatcher->Process( Input, m_pUsrEnv, &mTaskLogVis );

  // write the current time, i.e., the "StimulusTime" into the state vector
  State( "StimulusTime" ) = BCITIME::GetBCItime_ms();
  State( "PressedKey" ) = GetPressedKey();

  *Output = *Input;
}

