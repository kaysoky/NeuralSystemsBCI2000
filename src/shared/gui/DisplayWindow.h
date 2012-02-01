////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A GraphDisplay descendant which is a frameless GUI window for
//   an application's user display.
//
// $BEGIN_BCI2000_LICENSE$
// 
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2012: BCI2000 team and many external contributors ]
// 
// BCI2000 is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
// 
// BCI2000 is distributed in the hope that it will be useful, but
//                         WITHOUT ANY WARRANTY
// - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.
// 
// $END_BCI2000_LICENSE$
////////////////////////////////////////////////////////////////////////////////
#ifndef DISPLAY_WINDOW_H
#define DISPLAY_WINDOW_H

#include "GraphDisplay.h"

#ifdef __BORLANDC__
# include "VCL.h"
#else // __BORLANDC__
# include <QWidget>
#endif // __BORLANDC__

namespace GUI {

class DisplayWindow : public GraphDisplay
{
 public:
  DisplayWindow();
  virtual ~DisplayWindow();

  // Properties
  DisplayWindow& SetTitle( const std::string& );
  const std::string& Title() const;
  DisplayWindow& SetLeft( int );
  int Left() const;
  DisplayWindow& SetTop( int );
  int Top() const;
  DisplayWindow& SetWidth( int );
  int Width() const;
  DisplayWindow& SetHeight( int );
  int Height() const;
  DisplayWindow& Show();
  DisplayWindow& Hide();
  bool Visible() const;

 private:
  DisplayWindow& Restore();
  DisplayWindow& Clear();
  DisplayWindow& UpdateContext();
  
  std::string mTitle;
  int mTop,
      mLeft,
      mWidth,
      mHeight;

#ifdef __BORLANDC__
  HDC mWinDC;

  class TDisplayForm : public TForm
  {
   public:
    TDisplayForm( GraphDisplay& inDisplay )
      : TForm( ( TComponent* )NULL, 1 ),
        mUpdateRgn( ::CreateRectRgn( 0, 0, 0, 0 ) ),
        mDisplay( inDisplay )
      {
        Visible = false;
        BorderStyle = bsNone;
        // accessing the Handle property works around an apparent form
        // positioning bug in some recent VCL versions
        Handle;
        OnPaint = FormPaint;
        OnMouseDown = FormMouseDown;
      }
    __fastcall ~TDisplayForm()
      { ::DeleteObject( mUpdateRgn ); }

   private:
    // To avoid flicker and save memory bandwidth, use a WM_ERASEBKGND
    // handler that does not do anything.
    void __fastcall WMEraseBkgnd( TWMEraseBkgnd& )
      {}
    // Obtain the window's update region before BeginPaint() in the VCL
    // paint handler destroys (validates) it.
    void __fastcall WMPaint( TWMPaint& Message )
      {
        ::GetUpdateRgn( Handle, mUpdateRgn, false );
        PaintHandler( Message );
      }
    void __fastcall FormPaint( TObject* )
      { mDisplay.Paint( mUpdateRgn ); }

    void __fastcall FormMouseDown( TObject*, TMouseButton inButton, TShiftState, int inX, int inY )
      { if( inButton == mbLeft ) mDisplay.Click( inX, inY ); }

    BEGIN_MESSAGE_MAP
      VCL_MESSAGE_HANDLER( WM_PAINT, TWMPaint, WMPaint )
      VCL_MESSAGE_HANDLER( WM_ERASEBKGND, TWMEraseBkgnd, WMEraseBkgnd )
    END_MESSAGE_MAP( TForm )

   private:
    HRGN          mUpdateRgn;
    GraphDisplay& mDisplay;
  }* mpForm;

#else // __BORLANDC__

  QWidget* mpForm;

#endif // __BORLANDC__

};

} // namespace GUI

#endif // DISPLAY_WINDOW_H
