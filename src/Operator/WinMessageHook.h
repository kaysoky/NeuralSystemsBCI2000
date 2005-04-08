//////////////////////////////////////////////////////////////////////
//
// File: WinMessageHook.h
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
#ifndef WinMessageHookH
#define WinMessageHookH

#include <windows.h>

class WinMessageHook
{
 public:
  WinMessageHook();
  ~WinMessageHook();

 private:
  static LRESULT CALLBACK HookProc( int, WPARAM, LPARAM );
  
  static HHOOK          sHandle;
  static WinMessageHook sInstance;
};

#endif // GetMessageHookH
