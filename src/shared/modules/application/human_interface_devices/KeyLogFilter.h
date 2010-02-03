////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A filter that logs keyboard and mouse keypresses into states.
//   Useful for experiments that require tracking of user responses.
//   The PressedKey states contain the keys' "virtual key
//   code" as defined by the Win32 API.
//   In the MouseKeys state, bit 0 represents left and bit 1 represents the
//   right mouse button.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef KEY_LOG_FILTER_H
#define KEY_LOG_FILTER_H

#include "GenericFilter.h"
#include "Windows.h"
#include <string>

class KeyLogFilter : public GenericFilter
{
 enum
 {
   NumKeyStates = 3,

   KeyboardBits = 8,
   LButtonBit = 0,
   RButtonBit,

   KeyboardMask = ( 1 << KeyboardBits ) - 1,
   LButtonMask = 1 << LButtonBit,
   RButtonMask = 1 << RButtonBit,
 };

 public:
          KeyLogFilter();
  virtual ~KeyLogFilter();
  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize( const SignalProperties&, const SignalProperties& );
  virtual void StartRun();
  virtual void Process( const GenericSignal&, GenericSignal& );
  virtual void StopRun();
  virtual void Halt();
  virtual bool AllowsVisualization() const { return false; }

 private:
  bool mLogKeyPresses;

 private:
  static std::string KeyStateName( int );
  static void InstallHooks();
  static void UninstallHooks();
  static LRESULT CALLBACK LowLevelKeyboardProc( int, WPARAM, LPARAM );
  static LRESULT CALLBACK LowLevelMouseProc( int, WPARAM, LPARAM );

  static unsigned int sKeyPressed[ NumKeyStates ];
  static bool  sKeyReleased[ NumKeyStates ],
               sLButtonPressed,
               sLButtonReleased,
               sRButtonPressed,
               sRButtonReleased;
  static HHOOK sKeyboardHook,
               sMouseHook;
};

#endif // KEY_LOG_FILTER_H


