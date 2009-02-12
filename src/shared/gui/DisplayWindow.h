////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A GraphDisplay descendant which is a frameless GUI window for
//   an application's user display.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef DISPLAY_WINDOW_H
#define DISPLAY_WINDOW_H

#include "GraphDisplay.h"

#ifdef __BORLANDC__
# include "VCL.h"
#endif // __BORLANDC__

namespace GUI {

class DisplayWindow : public GraphDisplay
{
 public:
  DisplayWindow();
  virtual ~DisplayWindow();

  // Properties
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

#ifdef __BORLANDC__
 private:
  DisplayWindow& Restore();
  DisplayWindow& Clear();
  DisplayWindow& UpdateContext();
  
  int   mTop,
        mLeft,
        mWidth,
        mHeight;
  void* mWinDC;

  class TDisplayForm : public TForm
  {
   public:
    TDisplayForm( GraphDisplay& inDisplay )
      : TForm( ( TComponent* )NULL, 1 ),
        mUpdateRgn( ::CreateRectRgn( 0, 0, 0, 0 ) ),
        mDisplay( inDisplay )
      {
        Visible = false;
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
#endif // __BORLANDC__
};

} // namespace GUI

#endif // DISPLAY_WINDOW_H
