/////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: pbrunner@wadsworth.org
// Description: The mouse filter captures the mouse position on the screen
//   in device coordinates. The coordinates are always recorded.
//   State Variables:
//     Each cursor position is stored in device coordinates (i.e. coordinates
//     that are in units of screen pixels):
//       CursorPosX
//       CursorPosY
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
/////////////////////////////////////////////////////////////////////////////
#ifndef MOUSE_FILTER_H
#define MOUSE_FILTER_H

#include "GenericFilter.h"

class MouseFilter : public GenericFilter
{
 public:
          MouseFilter();
  virtual ~MouseFilter();
  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize( const SignalProperties&, const SignalProperties& );
  virtual void Process( const GenericSignal&, GenericSignal& );
  virtual bool AllowsVisualization() const { return false; }

 private:
  int  m_last_value_x;
  int  m_last_value_y;
  bool m_mouseworking;
  int  m_nBlockMissed;
};

#endif // MOUSE_FILTER_H




