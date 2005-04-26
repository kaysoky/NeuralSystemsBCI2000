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

#include <vcl.h>
#include <windows.h>
#include <math>
#include <algorithm>

// The hook will be installed from the constructor at static initialization time,
// i.e. from the main thread before WinMain is executed.
WinMessageHook WinMessageHook::sInstance;
HHOOK          WinMessageHook::sHookHandle = NULL;
DWORD          WinMessageHook::sThreadId = NULL;


WinMessageHook::WinMessageHook()
{
  if( sHookHandle == NULL )
  {
    sThreadId = ::GetCurrentThreadId();
    sHookHandle = ::SetWindowsHookEx( WH_GETMESSAGE, reinterpret_cast<HOOKPROC>( HookProc ),
      static_cast<HINSTANCE>( NULL ), sThreadId );
  }
}

WinMessageHook::~WinMessageHook()
{
  ::UnhookWindowsHookEx( sHookHandle );
  sHookHandle = NULL;
}

LRESULT CALLBACK
WinMessageHook::HookProc( int nCode, WPARAM wParam, LPARAM lParam )
{
  const int cMovePixels = 10;
  const int cMaxCoord = 1 << 16;

  MSG& ioMsg = *( reinterpret_cast<MSG*>( lParam ) );
  // Don't intercept messages for windows that don't belong to the main thread.
  if( ::GetWindowThreadProcessId( ioMsg.hwnd, NULL ) == sThreadId )
  {
    static TPoint originalMousePos = TPoint( -1, -1 );
    switch( ioMsg.message )
    {
      // Whenever the user clicks on the title bar of a window, we simulate
      // a mouse movement, forcing the DefWindowProc out of a modal loop
      // that would otherwise block the application.
      case WM_NCLBUTTONDOWN:
        if( !::IsZoomed( ioMsg.hwnd ) && ioMsg.wParam == HTCAPTION )
        {
          originalMousePos = Mouse->CursorPos;
          float x = ( ( Mouse->CursorPos.x + cMovePixels ) * cMaxCoord ) / float( Screen->Width ),
                y = ( Mouse->CursorPos.y * cMaxCoord ) / float( Screen->Height );
          ::mouse_event( MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE,
                                        ::ceil( x ), ::ceil( y ), 0, 0 );
        }
        break;
      // The WM_MOUSEMOVE message will appear in the message queue as a result
      // of the mouse event triggered for the WM_NCLBUTTONDOWN message.
      // It is then safe to move the mouse pointer back to its original position.
      case WM_MOUSEMOVE:
        if( originalMousePos.x != -1 )
        {
          float x = ( originalMousePos.x * cMaxCoord ) / float( Screen->Width ),
                y = ( originalMousePos.y * cMaxCoord ) / float( Screen->Height );
          ::mouse_event( MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE,
                                        ::ceil( x ), ::ceil( y ), 0, 0 );
          originalMousePos = TPoint( -1, -1 );
        }
        break;
      // Clicking the right mouse button would bring up a blocking menu.
      case WM_NCRBUTTONDOWN:
      case WM_NCMBUTTONDOWN:
        ioMsg.message = WM_NULL;
        break;
    }
  }
  return ::CallNextHookEx( sHookHandle, nCode, wParam, lParam );
}

