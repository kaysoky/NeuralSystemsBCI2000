//////////////////////////////////////////////////////////////////////
//
// File: WinMessageHook.cpp
//
// Date: Apr 06, 2005
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// Description: This file installs a GetMessage/PeekMessage hook
//    that modifies the behavior of windows when clicking their
//    title bar.
//    This keeps operator windows from blocking
//    the entire BCI2000 system when the user clicks a title bar.
//
///////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "WinMessageHook.h"

#include <windows.h>

// The hook will be installed from the constructor at static initialization time,
// i.e. from the main thread before WinMain is executed.
static WinMessageHook sHookInstance;
HHOOK WinMessageHook::sHandle = NULL;

WinMessageHook::WinMessageHook()
{
  sHandle = ::SetWindowsHookEx( WH_GETMESSAGE, reinterpret_cast<HOOKPROC>( HookProc ),
    static_cast<HINSTANCE>( NULL ), ::GetCurrentThreadId() );
}

WinMessageHook::~WinMessageHook()
{
  ::UnhookWindowsHookEx( sHandle );
}

LRESULT CALLBACK
WinMessageHook::HookProc( int nCode, WPARAM wParam, LPARAM lParam )
{
  const int cOffsetMickeys = 3;
  static int mouseOffset = 0;
  MSG& ioMsg = *( reinterpret_cast<MSG*>( lParam ) );
  switch( ioMsg.message )
  {
    // Whenever the user clicks on the title bar of a window, we simulate
    // a mouse movement, forcing the DefWindowProc out of a modal loop
    // that would otherwise block the application.
    case WM_NCLBUTTONDOWN:
      if( !::IsZoomed( ioMsg.hwnd ) && ioMsg.wParam == HTCAPTION )
      {
        ::mouse_event( MOUSEEVENTF_MOVE, cOffsetMickeys, 0, 0, 0 );
        mouseOffset -= cOffsetMickeys;
      }
      break;
    // The WM_MOUSEMOVE message will appear in the message queue as a result
    // of the mouse event triggered for the WM_NCLBUTTONDOWN message.
    // It is then safe to move the mouse pointer back to its original position.
    case WM_MOUSEMOVE:
      if( mouseOffset != 0 )
      {
        ::mouse_event( MOUSEEVENTF_MOVE, mouseOffset, 0, 0, 0 );
        mouseOffset = 0;
      }
      break;
    // Clicking the right mouse button would bring up a blocking menu.
    case WM_NCRBUTTONDOWN:
    case WM_NCMBUTTONDOWN:
      ioMsg.message = WM_NULL;
      break;
  }
  return ::CallNextHookEx( sHandle, nCode, wParam, lParam );
}

