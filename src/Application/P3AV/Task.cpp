#pragma hdrstop
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dos.h>
#include "PCHIncludes.h"
#include "Task.h"
#include "UBCITime.h"
#include "UCoreComm.h"
#include "UGenericVisualization.h"
#include "UsrEnv.h"
#include "UsrEnvDispatcher.h"
#include "UsrEnvAlgorithmP3AV.h"
#include "WavePlayer.h"

RegisterFilter( TTask, 3 );

// **************************************************************************
// Function:   TASK
// Purpose:    This is the constructor for the TASK class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
TTask::TTask()
            : m_pGenericVisualization( NULL ) ,
              m_pUsrEnvDispatcher( new UsrEnvDispatcher(Parameters, States) ),
              m_pUsrEnv( new UsrEnv(AnsiString("P3AVTask"), new UsrEnvAlgorithmP3AV) ),
              m_pBCITime( new BCITIME ) 
{

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
      "Background Color in hex (0x00BBGGRR)",
  END_PARAMETER_DEFINITIONS

  // Stimulus parameters
  BEGIN_PARAMETER_DEFINITIONS
    "P3AV_Stimulus int StimulusWidth= 5 0 0 100 // "
      "StimulusWidth in percent of screen width",
    "P3AV_Stimulus int CaptionHeight= 10 0 0 100 // "
      "Height of stimulus caption text in percent of screen height",
    "P3AV_Stimulus string CaptionColor= 0x00FFFFFF 0x00505050 0x00000000 0x00000000 // "
      "Color of stimulus caption text in hex (0x00BBGGRR)",
    "P3AV_Stimulus float AudioVolume= 1 1 0 1 // "
      "Volume for audio playback",
    "P3AV_Stimulus int OnTime= 10 10 0 5000 // "
      "Duration during which the stimulus is presented in units of SampleBlocks",
    "P3AV_Stimulus int OffTime= 3 10 0 5000 // "
      "Duration of an inter-stimulus interval following stimulus in units of SampleBlocks",
    "P3AV_Stimulus int PreSequenceTime= 50 30 0 5000 // "
      "Duration during which the stimulus is presented in units of SampleBlocks",
    "P3AV_Stimulus int PostSequenceTime= 50 30 0 5000 // "
      "Duration of an inter-stimulus interval following stimulus in units of SampleBlocks",
    "P3AV_Stimulus int MinInterTime= 0 0 0 0 // "
      "Minimum time that will be randomly added to the inter-stimulus interval in units of SampleBlocks",
    "P3AV_Stimulus int MaxInterTime= 3 3 3 3 // "
      "Maximum time that will be randomly added to the inter-stimulus interval in units of SampleBlocks",
    "P3AV_Stimulus int AudioSwitch= 1 1 0 1 // "
      "Whether audio files will be presented (will not be presented individually if not defined)",
    "P3AV_Stimulus int VideoSwitch= 1 1 0 1 // "
      "Whether icons files will be presented (will not be presented individually if not defined)",
    "P3AV_Stimulus int CaptionSwitch= 1 1 0 1 // "
      "Whether captions will be presented (will not be presented individually if not defined)",
  END_PARAMETER_DEFINITIONS

  // stimuli sequence related parameters
  BEGIN_PARAMETER_DEFINITIONS
    "P3AV_Stimuli matrix Matrix= "
     "{ caption icon audio } " // row labels
     "{ stimulus1 stimulus2 stimulus3 stimulus4 stimulus5 stimulus6 } " // column labels
     " One Two Three Four Five Six "
     "icons\\1.bmp icons\\2.bmp icons\\3.bmp icons\\4.bmp icons\\5.bmp icons\\6.bmp "
     "sounds\\1.wav sounds\\2.wav sounds\\3.wav sounds\\4.wav sounds\\5.wav sounds\\6.wav  "
     " // captions and icons to be displayed, sounds to be played for different stimuli",

    "P3AV_Stimuli matrix FocusOn= "
     "{ caption icon audio } " // row labels
     "{ focuson } " // column labels
     "Please%20focus%20on "     // caption
     "icons\\focuson.bmp " // video
     "sounds\\uh-uh.wav  "  // audio
     " // initial announcement what to focus on",

    "P3AV_Stimuli matrix Result= "
     "{ caption icon audio } " // row labels
     "{ result } " // column labels
     "The%20result%20was "     // caption
     "icons\\result.bmp " // video
     "sounds\\uh-uh.wav "  // audio
     " // final result announcement ",

    "P3AV_Stimuli intlist Sequence= 4 1 3 4 2 1 1 1000 // "
      "Sequence in which stimuli are presented (deterministic mode)/ Stimulus frequencies for each stimulus (random mode)",

    "P3AV_Stimuli int SequenceType= 0 0 0 1 // "
      "Sequence of stimuli can be deterministic(0) or random(1)",

    "P3AV_Stimuli int NumberOfSeq= 3 1 0 100 // "
      "How many times the sequence will be played",

    "P3AV_Stimuli int InterpretMode= 0 0 0 2 // "
      "Classification of results can be in none(0), free(1) or copy(2) mode",

    "P3AV_Stimuli intlist ToBeCopied= 3 1 2 3 1 1 1000 // "
      "Sequence in which stimuli need to be copied (only used in copy mode)",

    "P3AV_Stimuli string UserComment= Add your comment here // "
      "User comments for a specific run",
  END_PARAMETER_DEFINITIONS

  BEGIN_STATE_DEFINITIONS
    "SelectedStimulus 7 0 0 0",
    "StimulusTime 16 17528 0 0",
    "PhaseInSequence 2 0 0 0",
    "StimulusCode 7 0 0 0",
    "StimulusType 1 0 0 0",
    "Flashing 1 0 0 0",
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
  if (m_pGenericVisualization != NULL) delete m_pGenericVisualization;
  if (m_pUsrEnv != NULL) delete m_pUsrEnv;
  if (m_pUsrEnvDispatcher != NULL) delete m_pUsrEnvDispatcher;
  if (m_pBCITime != NULL) delete m_pBCITime;

  m_pGenericVisualization = NULL;
  m_pUsrEnv = NULL;
  m_pUsrEnvDispatcher = NULL;
  m_pBCITime = NULL;
}


void TTask::Preflight(const SignalProperties& inputProperties,
                            SignalProperties& outputProperties ) const
{
  bool bError = false;
  std::vector< std::string > vParamNames;
  vParamNames.clear();
  // check all the parameters presence
  vParamNames.push_back("WinXpos");
  vParamNames.push_back("WinYpos");
  vParamNames.push_back("WinWidth");
  vParamNames.push_back("WinHeight");
  vParamNames.push_back("WinBackgroundColor");
  vParamNames.push_back("StimulusWidth");
  vParamNames.push_back("CaptionHeight");
  vParamNames.push_back("CaptionColor");
  vParamNames.push_back("AudioVolume");
  vParamNames.push_back("OnTime");
  vParamNames.push_back("OffTime");
  vParamNames.push_back("PreSequenceTime");
  vParamNames.push_back("PostSequenceTime");
  vParamNames.push_back("MinInterTime");
  vParamNames.push_back("MaxInterTime");
  vParamNames.push_back("AudioSwitch");
  vParamNames.push_back("VideoSwitch");
  vParamNames.push_back("CaptionSwitch");
  vParamNames.push_back("Matrix");
  vParamNames.push_back("FocusOn");
  vParamNames.push_back("Result");
  vParamNames.push_back("Sequence");
  vParamNames.push_back("SequenceType");
  vParamNames.push_back("NumberOfSeq");
  vParamNames.push_back("InterpretMode");
  vParamNames.push_back("ToBeCopied");
  vParamNames.push_back("UserComment");
  vParamNames.push_back("SampleBlockSize");
  vParamNames.push_back("NumSamplesInERP");

  for (unsigned int i(0); i < vParamNames.size(); ++i)
  {
    if ((GetParamPtr(vParamNames[i]) == NULL) == true)
    {
      bcierr << vParamNames[i].c_str() << " parameter is not set" << std::endl;
      bError = true;
    }
  }
  vParamNames.clear();
  if (bError == false)
  {
    if (atoi(GetParamPtr("OnTime")->GetValue()) == 0)
    {
      bcierr << "OnTime parameter can not be 0." << std::endl;
      bError = true;
    }

    if (atoi(GetParamPtr("InterpretMode")->GetValue()) < 0 ||
        atoi(GetParamPtr("InterpretMode")->GetValue()) > 2)
    {
      bcierr << "InterpretMode parameter can be only 0(none mode), 1(free mode), or 2(copy mode)" << std::endl;
      bError = true;
    }
    if (atoi(GetParamPtr("AudioSwitch")->GetValue()) < 0 ||
        atoi(GetParamPtr("AudioSwitch")->GetValue()) > 1)
    {
      bcierr << "AudioSwitch parameter can be only 0(off), 1(on)" << std::endl;
      bError = true;
    }
    if (atoi(GetParamPtr("VideoSwitch")->GetValue()) < 0 ||
        atoi(GetParamPtr("VideoSwitch")->GetValue()) > 1)
    {
      bcierr << "VideoSwitch parameter can be only 0(off), 1(on)" << std::endl;
      bError = true;
    }
    if (atoi(GetParamPtr("CaptionSwitch")->GetValue()) < 0 ||
        atoi(GetParamPtr("CaptionSwitch")->GetValue()) > 1)
    {
      bcierr << "CaptionSwitch parameter can be only 0(off), 1(on)" << std::endl;
      bError = true;
    }
    if (atoi(GetParamPtr("MinInterTime")->GetValue()) > atoi(GetParamPtr("MaxInterTime")->GetValue()))
    {
      bcierr << "MinInterTime parameter can not be larger than MaxInterTime parameter" << std::endl;
      bError = true;
    }
    if (atoi(GetParamPtr("PreSequenceTime")->GetValue()) < 2 * atoi(GetParamPtr("OnTime")->GetValue()) ||
        atoi(GetParamPtr("PostSequenceTime")->GetValue()) < 2 * atoi(GetParamPtr("OnTime")->GetValue()))
    {
      bcierr << "PreSequenceTime and PostSequenceTime parameters must be at least 2 times larger than OnTime parameter" << std::endl;
      bError = true;
    }
    if (atoi(GetParamPtr("PostSequenceTime")->GetValue()) * atoi(GetParamPtr("SampleBlockSize")->GetValue()) <
        atoi(GetParamPtr("NumSamplesInERP")->GetValue()))
    {
      bcierr << "NumSamplesInERP has to be less than (PostSequenceTime * SampleBlockSize)" << std::endl;
      bError = true;
    }
  }

  // check stimuli matrix , FocusOn matrix, Result matrix
  if (ErrorReadingMatrix("Matrix") || ErrorReadingMatrix("FocusOn") || ErrorReadingMatrix("Result"))
    bError = true;

  // now check the states
  std::vector< std::string > vStateNames;
  vStateNames.clear();
  vStateNames.push_back("SelectedStimulus");
  vStateNames.push_back("StimulusTime");
  vStateNames.push_back("PhaseInSequence");
  vStateNames.push_back("StimulusCode");
  vStateNames.push_back("StimulusType");
  vStateNames.push_back("Flashing");
  vStateNames.push_back("StimulusCodeRes");
  vStateNames.push_back("StimulusTypeRes");
  vStateNames.push_back("Running");

  if (Statevector != NULL)
    for (unsigned int i(0); i < vStateNames.size(); ++i)
    {
      if ((Statevector->GetStateListPtr()->GetStatePtr(vStateNames[i].c_str()) == NULL) == true)
      {
        bcierr << vStateNames[i].c_str() << " state is not set" << std::endl;
        bError = true;
      }
    }
  vStateNames.clear();

  // check whether sound card is present
  const int WaveOutputDeviceCount = (int)waveOutGetNumDevs();
  if (WaveOutputDeviceCount == 0)
  {
    bcierr << "Sound card is missing." << std::endl;
    bError = true;
  }

  if (bError)
    PreflightCondition( false );
  else
  {
    PreflightCondition( inputProperties >= SignalProperties( 1, 1 ) );
    outputProperties = SignalProperties( 0, 0 );
  }
} // Preflight


const bool TTask::ErrorReadingMatrix(const AnsiString asMatrixName) const
{
  for( size_t i = 0; i < Parameter(asMatrixName.c_str())->GetNumValuesDimension2(); ++i )
  {
    std::string applicationPath = ExtractFilePath(Application->ExeName).c_str();
    // check sound files
    if (atoi(Parameter( "AudioSwitch" )->GetValue()) == 1)
    {
      std::string soundFileName = ( const char* )Parameter(asMatrixName.c_str(), "audio", i );
      if (soundFileName != "")
         {
         soundFileName = applicationPath + soundFileName;
         if (ErrorLoadingAudioFile(soundFileName) == true)
            return true;
         }
    }

    // check icon files
    if (atoi(Parameter( "VideoSwitch" )->GetValue()) == 1)
    {
      std::string iconFileName = ( const char* )Parameter(asMatrixName.c_str(), "icon", i );
      if (iconFileName != "")
         {
         iconFileName = applicationPath + iconFileName;
         if (ErrorLoadingVideoFile(iconFileName) == true)
           return true;
         }
    }
  }
  return false;
}


const bool TTask::ErrorLoadingAudioFile(std::string sAudioFile) const
{
  TWavePlayer testPlayer;
  TWavePlayer::Error err;
  if (Parameter( "AudioSwitch" ) == 1)
    err = testPlayer.AttachFile(sAudioFile.c_str());
  if( err == TWavePlayer::fileOpeningError )
  {
    bcierr << "Could not open \"" << sAudioFile << "\" as a sound file" << std::endl;
    return true;
  }
  else if( err != TWavePlayer::noError )
  {
    bcierr << "Some general error prevents wave audio playback" << std::endl;
    return true;
  }
  return false;
} // ErrorLoadingAudioFile


const bool TTask::ErrorLoadingVideoFile(std::string sVideoFile) const
{
  TImage * pIcon = new TImage(Application);
  try
  {
    if (Parameter( "VideoSwitch" ) == 1)
      pIcon->Picture->LoadFromFile(sVideoFile.c_str());
  }
  catch(EInOutError & )
  {
    bcierr << "Input/Output error for \"" << sVideoFile << std::endl;
    delete pIcon;
    pIcon = NULL;
    return true;
  }
  catch(EInvalidGraphic & )
  {
    bcierr << "Invalid Graphic error for \"" << sVideoFile << std::endl;
    delete pIcon;
    pIcon = NULL;
    return true;
  }
  catch (EFOpenError &)
  {
    bcierr << "Opening error for \"" << sVideoFile << std::endl;
    delete pIcon;
    pIcon = NULL;
    return true;
  }
  catch(Exception &)
  {
    bcierr << "Error loading \"" << sVideoFile << std::endl;
    delete pIcon;
    pIcon = NULL;
    return true;
  }
  return false;
} // ErrorLoadingVideoFile


// **************************************************************************
// Function:   Initialize
// Purpose:    Initializes the task, e.g., resets the user env dispatcher, etc.
// Parameters: 
// Returns:    N/A
// **************************************************************************
void TTask::Initialize(void)
{
  if (m_pGenericVisualization != NULL)
  {
    delete m_pGenericVisualization;
    m_pGenericVisualization = NULL;
  }
  m_pGenericVisualization = new GenericVisualization;
  m_pGenericVisualization->SetSourceID(SOURCEID_TASKLOG);
  m_pGenericVisualization->SendCfg2Operator(SOURCEID_TASKLOG, CFGID_WINDOWTITLE, "User Task Log");

  // initialize user environment
  if (m_pUsrEnv != NULL)
  {
    int iWinXpos(5), iWinYpos(5), iWinWidth(512), iWinHeight(512);
    TColor backgroundColor = clDkGray;
    try
    {
      backgroundColor = (TColor)strtol( Parameter( "WinBackgroundColor" ), NULL, 16 );
      iWinXpos  = Parameter( "WinXpos" );
      iWinYpos  = Parameter( "WinYpos" );
      iWinWidth = Parameter( "WinWidth" );
      iWinHeight= Parameter( "WinHeight" );
    }
    catch( TooGeneralCatch& )
    {
    }
    m_pUsrEnv->Initialize(this, Application, iWinYpos, iWinXpos, iWinWidth, iWinHeight, backgroundColor);
  }

  // initialize user environment dispatcher
  if (m_pUsrEnvDispatcher != NULL)
  {
    m_pUsrEnvDispatcher->Initialize(Parameters, m_pUsrEnv, Statevector);
  }
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
  const std::vector< float > & signals = Input->GetChannel( 0 );

  // creates a new state of the user environment
  if (m_pUsrEnvDispatcher != NULL)
    m_pUsrEnvDispatcher->Process(signals, m_pUsrEnv, Statevector, m_pGenericVisualization);

  // write the current time, i.e., the "StimulusTime" into the state vector
  State( "StimulusTime" ) = m_pBCITime->GetBCItime_ms();
}

