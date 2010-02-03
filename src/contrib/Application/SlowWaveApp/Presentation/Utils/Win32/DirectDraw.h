/////////////////////////////////////////////////////////////////////////////
//
// File: DirectDraw.h
//
// Date: Mar 18, 2003
//
// Author: Juergen Mellinger
//
// Description: A class that encapsulates instantiation of DirectDraw interface
//              objects. There is one interface object for each display
//              attached to the desktop, accessible via a monitor handle,
//              cf the VCL help on TScreen::Monitors.
//              Write DDraw[ hMon ] to obtain a pointer to the DirectDraw
//              interface associated with that monitor.
//              Note that the pointer may be NULL if there is no associated
//              interface.
//
// Changes:
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////
#ifndef DirectDrawH
#define DirectDrawH

#include <DDraw.h>

class TDDraw : public std::map<HMONITOR, LPDIRECTDRAW7>
{
 public:
  // Constructor: Fill the set with DDraw interfaces, one
  // for each display device.
  TDDraw();

  // Destructor: Release all interface objects.
  ~TDDraw();

 private:
  static BOOL WINAPI AddDevice( GUID FAR*, LPSTR, LPSTR, LPVOID, HMONITOR );
  static HINSTANCE dllInstance;
  static size_t numInstances;
};

// The one global TDDraw instance.
extern TDDraw DDraw;

#endif // DirectDrawH
