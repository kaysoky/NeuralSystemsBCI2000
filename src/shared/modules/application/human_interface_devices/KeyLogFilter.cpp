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
#include "PCHIncludes.h"
#pragma hdrstop

#include "KeyLogFilter.h"

#include <sstream>
#include <windows.h>

using namespace std;

RegisterFilter( KeyLogFilter, 3.1 ); // Place it after the task filter.

unsigned int KeyLogFilter::sKeyPressed[ KeyLogFilter::NumKeyStates ] = { 0 };
bool KeyLogFilter::sKeyReleased[ KeyLogFilter::NumKeyStates ] = { false };
bool KeyLogFilter::sLButtonPressed = false;
bool KeyLogFilter::sLButtonReleased = false;
bool KeyLogFilter::sRButtonPressed = false;
bool KeyLogFilter::sRButtonReleased = false;
HHOOK KeyLogFilter::sKeyboardHook = NULL;
HHOOK KeyLogFilter::sMouseHook = NULL;

KeyLogFilter::KeyLogFilter()
: mLogKeyPresses( false )
{
  BEGIN_PARAMETER_DEFINITIONS
    "Application:Human%20Interface%20Devices int LogKeyPresses= 0 0 0 1 "
      "// log key presses into MouseKeys/PressedKey states (boolean)",
  END_PARAMETER_DEFINITIONS

  BEGIN_STATE_DEFINITIONS
    "MouseKeys 2 0 0 0",
  END_STATE_DEFINITIONS

  for( size_t i = 0; i < NumKeyStates; ++i )
  {
    ostringstream oss;
    oss << KeyStateName( i ) << " " << KeyboardBits << " 0 0 0";
    States->Add( oss.str() );
  }
}

KeyLogFilter::~KeyLogFilter()
{
  Halt();
}

void
KeyLogFilter::Preflight( const SignalProperties& Input,
                               SignalProperties& Output ) const
{
  for( size_t i = 0; i < NumKeyStates; ++i )
    State( KeyStateName( i ) );

  InstallHooks();
  if( sKeyboardHook == NULL )
    bciout << "Could not install windows keyboard hook" << endl;
  if( sMouseHook == NULL )
    bciout << "Could not install windows mouse hook" << endl;
  UninstallHooks();
  Output = Input;
}

void
KeyLogFilter::Initialize( const SignalProperties&,
                          const SignalProperties& )
{
  mLogKeyPresses = ( Parameter( "LogKeyPresses" ) != 0 );
}

void
KeyLogFilter::StartRun()
{
  for( size_t i = 0; i < NumKeyStates; ++i )
  {
    sKeyPressed[ i ] = 0;
    sKeyReleased[ i ] = false;
    State( KeyStateName( i ) ) = 0;
  }
  sLButtonPressed = false;
  sLButtonReleased = false;
  sRButtonPressed = false;
  sRButtonReleased = false;
  State( "MouseKeys" ) = 0;
  if( mLogKeyPresses )
    InstallHooks();
}

void
KeyLogFilter::Process( const GenericSignal& Input,
                             GenericSignal& Output )
{
  int stateValue = 0;
  if( sLButtonPressed )
    stateValue |= LButtonMask;
  if( sRButtonPressed )
    stateValue |= RButtonMask;
  State( "MouseKeys" ) = stateValue;
  if( stateValue != 0 )
    bcidbg << "MouseKeys: " << hex << "0x" << stateValue << endl;
  if( sLButtonReleased )
    sLButtonPressed = false;
  if( sRButtonReleased )
    sRButtonPressed = false;

  for( size_t i = 0; i < NumKeyStates; ++i )
  {
    int stateValue = sKeyPressed[ i ] & KeyboardMask;
    State( KeyStateName( i ) ) = stateValue;
    if( stateValue != 0 )
      bcidbg << KeyStateName( i ) << ": " << hex << "0x" << stateValue << endl;
    if( sKeyReleased[ i ] )
      sKeyPressed[ i ] = 0;
  }

  Output = Input;
}

void
KeyLogFilter::StopRun()
{
  UninstallHooks();
}

void
KeyLogFilter::Halt()
{
  UninstallHooks();
}


string
KeyLogFilter::KeyStateName( int i )
{
  ostringstream oss;;
  oss << "PressedKey" << i + 1;
  return oss.str();
}

void
KeyLogFilter::InstallHooks()
{
  HINSTANCE module = ::GetModuleHandle( NULL );
  if( sKeyboardHook == NULL )
    sKeyboardHook = ::SetWindowsHookEx( WH_KEYBOARD_LL,
      reinterpret_cast<HOOKPROC>( LowLevelKeyboardProc ), module, NULL );
  if( sMouseHook == NULL )
    sMouseHook = ::SetWindowsHookEx( WH_MOUSE_LL,
      reinterpret_cast<HOOKPROC>( LowLevelMouseProc ), module, NULL );
}


void
KeyLogFilter::UninstallHooks()
{
  if( sKeyboardHook != NULL )
  {
    ::UnhookWindowsHookEx( sKeyboardHook );
    sKeyboardHook = NULL;
  }
  if( sMouseHook != NULL )
  {
    ::UnhookWindowsHookEx( sMouseHook );
    sMouseHook = NULL;
  }
}


LRESULT CALLBACK
KeyLogFilter::LowLevelKeyboardProc( int inCode, WPARAM inWParam, LPARAM inLParam )
{
  if( inCode >= 0 )
  {
    KBDLLHOOKSTRUCT* pData = reinterpret_cast<KBDLLHOOKSTRUCT*>( inLParam );
    switch( inWParam )
    {
      case WM_KEYDOWN:
      case WM_SYSKEYDOWN:
        { // Check whether the key is already known to be pressed.
          size_t i = 0;
          while( i < NumKeyStates
                 && !( sKeyPressed[ i ] == pData->vkCode && sKeyReleased[ i ] == false ) )
            ++i;
          if( i == NumKeyStates )
          { // The key is newly pressed. This also happens if it was released
            // and pressed again during the same block.
            i = 0;
            while( i < NumKeyStates && sKeyPressed[ i ] != 0 )
              ++i;
            if( i < NumKeyStates )
            { // Log the virtual key code.
              sKeyPressed[ i ] = pData->vkCode;
              sKeyReleased[ i ] = false;
            }
          }
        }
        break;

      case WM_KEYUP:
      case WM_SYSKEYUP:
        { // Is the key known to be pressed?
          size_t i = 0;
          while( i < NumKeyStates && sKeyPressed[ i ] != pData->vkCode )
            ++i;
          if( i < NumKeyStates ) // Mark it as released.
            sKeyReleased[ i ] = true;
        }
        break;
        
      default:
        ;
    }
  }
  return ::CallNextHookEx( sKeyboardHook, inCode, inWParam, inLParam );
}


LRESULT CALLBACK
KeyLogFilter::LowLevelMouseProc( int inCode, WPARAM inWParam, LPARAM inLParam )
{
  if( inCode >= 0 )
  {
    switch( inWParam )
    {
      case WM_LBUTTONDOWN:
        sLButtonPressed = true;
        sLButtonReleased = false;
        break;
      case WM_LBUTTONUP:
        sLButtonReleased = true;
        break;
      case WM_RBUTTONDOWN:
        sRButtonPressed = true;
        sRButtonReleased = false;
        break;
      case WM_RBUTTONUP:
        sRButtonReleased = true;
        break;
      default:
        ;
    }
  }
  return ::CallNextHookEx( sMouseHook, inCode, inWParam, inLParam );
}

