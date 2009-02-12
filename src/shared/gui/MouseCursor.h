////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: juergen.mellinger@uni-tuebingen.de
// Description: A class template for objects that replace the mouse cursor
//   during their lifetime, i.e. from construction to destruction.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef MOUSE_CURSOR_H
#define MOUSE_CURSOR_H

#include "windows.h"

template<int CursorID> class MouseCursor
{
 public:
  MouseCursor()
    : mPrevCursor( ::SetCursor( ::LoadCursor( NULL, CursorID ) ) )
    {}
  ~MouseCursor()
    { ::SetCursor( mPrevCursor ); }
 private:
  HCURSOR mPrevCursor;
};

typedef MouseCursor<IDC_WAIT> HourglassCursor;

#endif MOUSE_CURSOR_H
