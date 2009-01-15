//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: This file installs a GetMessage/PeekMessage hook
//    that modifies the behavior of windows when clicking their
//    title bar.
//    This keeps operator windows from blocking
//    the entire BCI2000 system when the user clicks a title bar.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
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

  static HHOOK          sHookHandle;
  static DWORD          sThreadId;
  static WinMessageHook sInstance;
};

#endif // GetMessageHookH
