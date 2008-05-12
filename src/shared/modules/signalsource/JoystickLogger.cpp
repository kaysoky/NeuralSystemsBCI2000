/////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: pbrunner@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: The joystick filter supports joysticks with up to 3 axes
//   and 4 buttons that are interfaced with windows via the USB port.
//
// Parameterization:
//   Joystick logging is enabled from the command line adding
//   --LogJoystick=1
//   as a command line option.
//
// State Variables:
//   Each position state is ranging from -16348 to +16347 with a resting
//   position at 0. Each button state is either 1 when pressed or 0 when
//   not pressed.
//   JoystickXpos
//   JoystickYpos
//   JoystickZpos
//   JoystickButtons1
//   JoystickButtons2
//   JoystickButtons3
//   JoystickButtons4
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
/////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "JoystickLogger.h"
#include "BCIEvent.h"

#define MAXJOYSTICK 32768

using namespace std;

static JoystickLogger sInstance;

// **************************************************************************
// Function:   JoystickLogger
// Purpose:    This is the constructor for the JoystickLogger class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
JoystickLogger::JoystickLogger()
: mpJoystickThread( NULL ),
  m_joystickenable( false )
{
}

// **************************************************************************
// Function:   ~JoystickLogger
// Purpose:    This is the destructor for the JoystickLogger class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
JoystickLogger::~JoystickLogger()
{
}

// **************************************************************************
// Function:   Publish
// Purpose:    This function requests parameters by adding parameters to the
//             parameter list it also requests states by adding them to the
//             state list
// Returns:    N/A
// **************************************************************************
void JoystickLogger::Publish()
{
  if( OptionalParameter( "LogJoystick" ) > 0 )
  {
    BEGIN_PARAMETER_DEFINITIONS
      "Source:Log%20Input int LogJoystick= 1 0 0 1 "
      " // record joystick to states (boolean)",
    END_PARAMETER_DEFINITIONS

    BEGIN_STATE_DEFINITIONS
      "JoystickXpos     16 0 0 0",
      "JoystickYpos     16 0 0 0",
      "JoystickZpos     16 0 0 0",
      "JoystickButtons1  1 0 0 0",
      "JoystickButtons2  1 0 0 0",
      "JoystickButtons3  1 0 0 0",
      "JoystickButtons4  1 0 0 0",
    END_STATE_DEFINITIONS
  }
}

// **************************************************************************
// Function:   Preflight
// Purpose:    Checks parameters for availability and consistency with
//             input signal properties; requests minimally needed properties for
//             the output signal; checks whether resources are available.
// Parameters: Input and output signal properties.
// Returns:    N/A
// **************************************************************************
void JoystickLogger::Preflight() const
{
  JOYINFO      joyinfo;
  JOYCAPS      joycaps;
  MMRESULT     ret              = -1;
  bool         joystickenable   = false;
  unsigned int nNumFound        = 0;

  joystickenable = ( ( int )OptionalParameter( "LogJoystick" ) != 0 );
  if( joystickenable )
  {
    nNumFound = joyGetNumDevs();
    if (nNumFound == 0)
      bcierr << "No joystick driver installed" << endl;
    else
    {
      ret = joyGetPos( JOYSTICKID1, &joyinfo );
      if (ret != JOYERR_NOERROR)
      {
        switch( ret )
        {
          case MMSYSERR_NODRIVER:
            bcierr << "The joystick driver is not present" << endl;
            break;
          case MMSYSERR_INVALPARAM:
            bcierr << "The specified joystick identifier is invalid" << endl;
            break;
          case MMSYSERR_BADDEVICEID:
            bcierr << "The specified joystick identifier is invalid" << endl;
            break;
          case JOYERR_UNPLUGGED:
            bcierr << "The specified joystick is not connected to the system" << endl;
            break;
          case JOYERR_PARMS:
            bcierr << "The specified joystick identifier is invalid" << endl;
            break;
        }
      }
      else
      {
        ret = joyGetDevCaps( JOYSTICKID1,&joycaps,sizeof(joycaps));
        if (ret == JOYERR_NOERROR)
          bciout << "Joystick " << joycaps.szPname << " with " << joycaps.wNumAxes
                 << " axes and " << joycaps.wNumButtons << " buttons found"
                 << endl;
        else
          bcierr << "Could not obtain joystick capabilities" << endl;
      }
    }
  }
}

// **************************************************************************
// Function:   Initialize
// Purpose:    This function parameterizes the JoystickLogger
// Parameters: Input and output signal properties.
// Returns:    N/A
// **************************************************************************
void JoystickLogger::Initialize()
{
  MMRESULT ret = -1;

  m_joystickenable = ( ( int )OptionalParameter( "LogJoystick" ) != 0 );

  if( m_joystickenable )
  {
    ret = joyGetDevCaps( JOYSTICKID1,&m_joycaps,sizeof(m_joycaps));
  }
}

// **************************************************************************
// Function:   StartRun
// Purpose:    Starts a new Joystick thread at the beginning of a run
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void JoystickLogger::StartRun()
{
  if( m_joystickenable )
    mpJoystickThread = new JoystickThread( m_joycaps );
}

// **************************************************************************
// Function:   StopRun
// Purpose:    Terminates the Joystick thread at the end of a run
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void JoystickLogger::StopRun()
{
  if( mpJoystickThread )
  {
    mpJoystickThread->Terminate();
    while( !mpJoystickThread->IsTerminated() )
      Sleep(1);
    delete mpJoystickThread;
    mpJoystickThread = NULL;
  }
}

// **************************************************************************
// Function:   Halt
// Purpose:    Stops all asynchronous operation
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void JoystickLogger::Halt()
{
  StopRun();
}

// **************************************************************************
// Function:   JoystickThread constructor
// Purpose:    Initializes the Joystick thread
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
JoystickLogger::JoystickThread::JoystickThread( const JOYCAPS& inJoycaps )
: OSThread( false ),
  m_joycaps( inJoycaps ),
  m_xPos( 0 ),
  m_yPos( 0 ),
  m_zPos( 0 ),
  m_button1( false ),
  m_button2( false ),
  m_button3( false ),
  m_button4( false ),
  m_prevXPos( 0 ),
  m_prevYPos( 0 ),
  m_prevZPos( 0 ),
  m_prevButton1( false ),
  m_prevButton2( false ),
  m_prevButton3( false ),
  m_prevButton4( false )
{
}

// **************************************************************************
// Function:   JoystickThread destructor
// Purpose:    Cleans up the Joystick thread
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
JoystickLogger::JoystickThread::~JoystickThread()
{
}

// **************************************************************************
// Function:   Execute
// Purpose:    This is the Joystick thread function
// Parameters: N/A
// Returns:    Always zero
// **************************************************************************
int JoystickLogger::JoystickThread::Execute()
{
  while( !IsTerminating() )
  {
    JOYINFO joyinfo;
    MMRESULT ret = joyGetPos( JOYSTICKID1, &joyinfo );
    if (ret == JOYERR_NOERROR)
    {
      m_xPos = ((float)(joyinfo.wXpos - m_joycaps.wXmin) / (float)(m_joycaps.wXmax - m_joycaps.wXmin) * MAXJOYSTICK); //- MAXJOYSTICK/2;
      m_yPos = ((float)(joyinfo.wYpos - m_joycaps.wYmin) / (float)(m_joycaps.wYmax - m_joycaps.wYmin) * MAXJOYSTICK); //- MAXJOYSTICK/2;
      m_zPos = ((float)(joyinfo.wZpos - m_joycaps.wZmin) / (float)(m_joycaps.wZmax - m_joycaps.wZmin) * MAXJOYSTICK); //- MAXJOYSTICK/2;
      m_button1 = (joyinfo.wButtons & JOY_BUTTON1) == JOY_BUTTON1;
      m_button2 = (joyinfo.wButtons & JOY_BUTTON2) == JOY_BUTTON2;
      m_button3 = (joyinfo.wButtons & JOY_BUTTON3) == JOY_BUTTON3;
      m_button4 = (joyinfo.wButtons & JOY_BUTTON4) == JOY_BUTTON4;
    }
    else
    {
      m_xPos = 0;
      m_yPos = 0;
      m_zPos = 0;
      m_button1 = false;
      m_button2 = false;
      m_button3 = false;
      m_button4 = false;
    }

    if( m_xPos != m_prevXPos )
      bcievent << "JoystickXpos " << m_xPos;
    if( m_yPos != m_prevYPos )
      bcievent << "JoystickYpos " << m_yPos;
    if( m_zPos != m_prevZPos )
      bcievent << "JoystickZpos " << m_zPos;
    if( m_button1 != m_prevButton1 )
      bcievent << "JoystickButtons1 " << m_button1;
    if( m_button2 != m_prevButton2 )
      bcievent << "JoystickButtons2 " << m_button2;
    if( m_button3 != m_prevButton3 )
      bcievent << "JoystickButtons3 " << m_button3;
    if( m_button4 != m_prevButton4 )
      bcievent << "JoystickButtons4 " << m_button4;

    m_prevXPos = m_xPos;
    m_prevYPos = m_yPos;
    m_prevZPos = m_zPos;
    m_prevButton1 = m_button1;
    m_prevButton2 = m_button2;
    m_prevButton3 = m_button3;
    m_prevButton4 = m_button4;

    Sleep(1);
  }
  return 0;
}

