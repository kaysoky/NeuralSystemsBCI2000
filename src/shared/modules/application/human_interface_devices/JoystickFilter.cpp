/////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: pbrunner@wadsworth.org
// Description: The joystick filter supports joysticks with up to 3 axes
//   and 4 buttons that are interfaced with windows via the USB port.
//
// Parameterization:
//   The joystick is configured in the panel HumanInterfaceDevices in the
//   section JoystickFilter.
//   JoystickEnable   = checked/unchecked
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

#include "JoystickFilter.h"

#define MAXJOYSTICK 32768

RegisterFilter( JoystickFilter, 3.0 );

// **************************************************************************
// Function:   JoystickFilter
// Purpose:    This is the constructor for the JoystickFilter class
//             it requests parameters by adding parameters to the parameter list
//             it also requests states by adding them to the state list
// Returns:    N/A
// **************************************************************************
JoystickFilter::JoystickFilter()
{

  BEGIN_PARAMETER_DEFINITIONS
    "Application:Human%20Interface%20Devices int JoystickEnable=             0 0 0 1 "
        "// enable recording from the joystick (0=no, 1=yes) (boolean)",
  END_PARAMETER_DEFINITIONS

  BEGIN_STATE_DEFINITIONS
    "JoystickXpos 16 0 0 0",
    "JoystickYpos 16 0 0 0",
    "JoystickZpos 16 0 0 0",
    "JoystickButtons1 1 0 0 0",
    "JoystickButtons2 1 0 0 0",
    "JoystickButtons3 1 0 0 0",
    "JoystickButtons4 1 0 0 0",
  END_STATE_DEFINITIONS

  m_joystickenable   = false;
  m_nNumFound        = 0;
  m_joystickworking  = false;
  m_nBlockMissed     = 0;


}

// **************************************************************************
// Function:   ~JoystickFilter
// Purpose:    This is the destructor for the JoystickFilter class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
JoystickFilter::~JoystickFilter()
{
}

// **************************************************************************
// Function:   Preflight
// Purpose:    Checks parameters for availability and consistency with
//             input signal properties; requests minimally needed properties for
//             the output signal; checks whether resources are available.
// Parameters: Input and output signal properties.
// Returns:    N/A
// **************************************************************************
void JoystickFilter::Preflight( const SignalProperties& Input,
                                      SignalProperties& Output ) const
{

  JOYINFO      joyinfo;
  JOYCAPS      joycaps;
  MMRESULT     ret              = -1;
  bool         joystickenable   = false;
  unsigned int nNumFound        = 0;

  joystickenable = ( ( int )Parameter( "JoystickEnable" ) != 0 );

  if (!joystickenable) {

    ret = joyGetDevCaps( JOYSTICKID1,&joycaps,sizeof(joycaps));

    if (ret == JOYERR_NOERROR) {
      bciout << "Joystick " << joycaps.szPname << " with " << joycaps.wNumAxes << " axes and " << joycaps.wNumButtons << " buttons found" << std::endl;
      bciout << "Recording form Joystick is disabled. To enable recording check JoystickEnable in section HumanInterfaceDevices" << std::endl;
    }

  } else {

    nNumFound = joyGetNumDevs();

    if (nNumFound == 0) {
      bcierr << "No joystick found" << std::endl;
      return;
    }

    ret = joyGetPos( JOYSTICKID1, &joyinfo );

    if (ret != JOYERR_NOERROR) {

      if (ret == MMSYSERR_NODRIVER) {
        bcierr << "The joystick driver is not present" << std::endl;
      }

      if (ret == MMSYSERR_INVALPARAM) {
        bcierr << "The specified joystick identifier is invalid" << std::endl;
      }

      if (ret == MMSYSERR_BADDEVICEID) {
        bcierr << "The specified joystick identifier is invalid" << std::endl;
      }

      if (ret == JOYERR_UNPLUGGED) {
        bcierr << "The specified joystick is not connected to the system" << std::endl;
      }

      if (ret == JOYERR_PARMS) {
        bcierr << "The specified joystick identifier is invalid" << std::endl;
      }

      return;

    } else {

      ret = joyGetDevCaps( JOYSTICKID1,&joycaps,sizeof(joycaps));

      if (ret == JOYERR_NOERROR) {

//        bciout << "Joystick " << joycaps.szPname << " with " << joycaps.wNumAxes << " axes and " << joycaps.wNumButtons << " buttons found" << std::endl;

      }


    }

  }

  Output = Input;
}

// **************************************************************************
// Function:   Initialize
// Purpose:    This function parameterizes the JoystickFilter
// Parameters: Input and output signal properties.
// Returns:    N/A
// **************************************************************************
void JoystickFilter::Initialize( const SignalProperties&, const SignalProperties& )
{
  MMRESULT ret = -1;

  m_joystickenable = ( ( int )Parameter( "JoystickEnable" ) != 0 );

  if (!m_joystickenable) {

    ret = joyGetDevCaps( JOYSTICKID1,&m_joycaps,sizeof(m_joycaps));

    if (ret == JOYERR_NOERROR) {
      bciout << "Joystick " << m_joycaps.szPname << " with " << m_joycaps.wNumAxes << " axes and " << m_joycaps.wNumButtons << " buttons found" << std::endl;
      bciout << "Recording form Joystick is disabled. To enable recording check JoystickEnable in section HumanInterfaceDevices" << std::endl;
    }

  } else {

    m_nNumFound = joyGetNumDevs();

    if (m_nNumFound == 0) {
      bcierr << "No joystick found" << std::endl;
      return;
    }

    ret = joyGetPos( JOYSTICKID1, &m_joyinfo );

    if (ret != JOYERR_NOERROR) {

      if (ret == MMSYSERR_NODRIVER) {
        bcierr << "The joystick driver is not present" << std::endl;
      }

      if (ret == MMSYSERR_INVALPARAM) {
        bcierr << "The specified joystick identifier is invalid" << std::endl;
      }

      if (ret == MMSYSERR_BADDEVICEID) {
        bcierr << "The specified joystick identifier is invalid" << std::endl;
      }

      if (ret == JOYERR_UNPLUGGED) {
        bcierr << "The specified joystick is not connected to the system" << std::endl;
      }

      if (ret == JOYERR_PARMS) {
        bcierr << "The specified joystick identifier is invalid" << std::endl;
      }

      return;

    } else {

      ret = joyGetDevCaps( JOYSTICKID1,&m_joycaps,sizeof(m_joycaps));

      if (ret == JOYERR_NOERROR) {

        bcidbg << "Joystick " << m_joycaps.szPname << " with " << m_joycaps.wNumAxes << " axes and " << m_joycaps.wNumButtons << " buttons found" << std::endl;

        m_joystickworking = true;

      }

    }

  }

}

// **************************************************************************
// Function:   Process
// Purpose:    This function applies the calibration routine
// Parameters: input  - input signal,
//             output - output signal for this filter
// Returns:    N/A
// **************************************************************************
void JoystickFilter::Process( const GenericSignal& Input, GenericSignal& Output )
{
  MMRESULT ret = -1;

  if (m_joystickenable) {

    ret = joyGetPos( JOYSTICKID1, &m_joyinfo );

    if (ret == JOYERR_NOERROR) {

      if (m_joystickworking == false) {
        bciout << "Joystick started working again after " << m_nBlockMissed << " blocks were missed" << std::endl;
        m_nBlockMissed = 0;
      }

      m_joystickworking = true;

    } else {

      if (m_joystickworking == true) {
        bciout << "Joystick stopped working" << std::endl;
      }

      m_joystickworking = false;
      m_nBlockMissed++;

    }


    State("JoystickXpos")     = (unsigned int)((float)(m_joyinfo.wXpos - m_joycaps.wXmin) / (float)(m_joycaps.wXmax - m_joycaps.wXmin) * MAXJOYSTICK); //- MAXJOYSTICK/2;
    State("JoystickYpos")     = (unsigned int)((float)(m_joyinfo.wYpos - m_joycaps.wYmin) / (float)(m_joycaps.wYmax - m_joycaps.wYmin) * MAXJOYSTICK); //- MAXJOYSTICK/2;
    State("JoystickZpos")     = (unsigned int)((float)(m_joyinfo.wZpos - m_joycaps.wZmin) / (float)(m_joycaps.wZmax - m_joycaps.wZmin) * MAXJOYSTICK); //- MAXJOYSTICK/2;
    State("JoystickButtons1") = (m_joyinfo.wButtons & JOY_BUTTON1) == JOY_BUTTON1;
    State("JoystickButtons2") = (m_joyinfo.wButtons & JOY_BUTTON2) == JOY_BUTTON2;
    State("JoystickButtons3") = (m_joyinfo.wButtons & JOY_BUTTON3) == JOY_BUTTON3;
    State("JoystickButtons4") = (m_joyinfo.wButtons & JOY_BUTTON4) == JOY_BUTTON4;

  } else {

    State("JoystickXpos")    = 0;
    State("JoystickYpos")    = 0;
    State("JoystickZpos")    = 0;
    State("JoystickButtons1") = 0;
    State("JoystickButtons2") = 0;
    State("JoystickButtons3") = 0;
    State("JoystickButtons4") = 0;

  }

  Output = Input;

}



