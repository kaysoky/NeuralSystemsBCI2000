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
#ifndef JOYSTICK_FILTER_H
#define JOYSTICK_FILTER_H

#include "GenericFilter.h"
#include <mmsystem.h>

class JoystickFilter : public GenericFilter
{
 public:
          JoystickFilter();
  virtual ~JoystickFilter();
  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize( const SignalProperties&, const SignalProperties& );
  virtual void Process( const GenericSignal&, GenericSignal& );
  virtual bool AllowsVisualization() const { return false; }

 private:
  JOYINFO      m_joyinfo;
  JOYCAPS      m_joycaps;
  bool         m_joystickenable;
  unsigned int m_nNumFound;
  bool         m_joystickworking;
  unsigned int m_nBlockMissed;



};

#endif // JOYSTICK_FILTER_H




