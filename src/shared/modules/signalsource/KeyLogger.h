////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A filter that logs keyboard and mouse keypresses into states.
//   Useful for experiments that require tracking of user responses.
//   A "Keyboard" state contains the key's "virtual key code" as defined by
//   the Win32 API, and additionally a bit which is set when a key is released.
//   In the "MouseKeys" state, bit 0 represents left and bit 1 represents the
//   right mouse button.
//   In the MousePosX and MousePosY states, mouse cursor position is stored in
//   device coordinates (i.e. coordinates that are in units of screen pixels)
//   with an additional offset of 32768 to cover negative coordinates.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef KEY_LOGGER_H
#define KEY_LOGGER_H

#include "Environment.h"
#include "OSThread.h"
#include <windows.h>

class KeyLogger : public EnvironmentExtension
{
 public:

 enum
 {
   KeyboardBits = 8,

   LButtonBit = 0,
   RButtonBit = 1,

   LButtonMask = 1 << LButtonBit,
   RButtonMask = 1 << RButtonBit,
 };

          KeyLogger();
  virtual ~KeyLogger();
  virtual void Publish();
  virtual void Preflight() const;
  virtual void Initialize();
  virtual void StartRun();
  virtual void StopRun();
  void Halt();

 private:
  bool mLogKeyboard;
  bool mLogMouse;

  class HookThread : public OSThread
  {
   public:
    HookThread( bool logKeyboard, bool logMouse );
    virtual ~HookThread();

   private:
    virtual int Execute();
    bool InstallKeyboardHook();
    bool InstallMouseHook();
    void UninstallHooks();

    static LRESULT CALLBACK LowLevelKeyboardProc( int, WPARAM, LPARAM );
    static LRESULT CALLBACK LowLevelMouseProc( int, WPARAM, LPARAM );

    static int  sInstances;
    static bool sKeyPressed[ 1 << KeyboardBits ];
    static int  sMouseKeys;
    static HHOOK sKeyboardHook,
                 sMouseHook;

    bool  mLogKeyboard,
          mLogMouse;
  };
  HookThread* mpThread;
};

#endif // KEY_LOGGER_H


