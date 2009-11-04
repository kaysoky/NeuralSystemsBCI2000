/////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: pbrunner@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: The joystick logger supports joysticks with up to 3 axes
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
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
/////////////////////////////////////////////////////////////////////////////
#ifndef JOYSTICK_LOGGER_H
#define JOYSTICK_LOGGER_H

#include "Environment.h"
#include "OSThread.h"
#include <mmsystem.h>

class JoystickLogger : public EnvironmentExtension
{
 public:
          JoystickLogger();
  virtual ~JoystickLogger();
  virtual void Publish();
  virtual void Preflight() const;
  virtual void Initialize();
  virtual void StartRun();
  virtual void StopRun();
  virtual void Halt();

 private:
  bool    m_joystickenable;
  JOYCAPS m_joycaps;

  class JoystickThread : public OSThread
  {
   public:
            JoystickThread( const JOYCAPS& );
    virtual ~JoystickThread();
    virtual int Execute();
    void GetJoyPos( unsigned int& x, unsigned int& y, unsigned int& z,
                    unsigned int& b1, unsigned int& b2, unsigned int& b3, unsigned int& b4 );

   private:
    JOYCAPS      m_joycaps;
    unsigned int m_prevXPos,
                 m_prevYPos,
                 m_prevZPos,
                 m_prevButton1,
                 m_prevButton2,
                 m_prevButton3,
                 m_prevButton4;

  } *mpJoystickThread;
};

#endif // JOYSTICK_LOGGER_H




