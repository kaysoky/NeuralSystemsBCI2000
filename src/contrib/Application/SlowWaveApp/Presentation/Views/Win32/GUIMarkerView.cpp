/////////////////////////////////////////////////////////////////////////////
//
// File: GUIMarkerView.cpp
//
// Date: Feb 16, 2003
//
// Author: Juergen Mellinger
//
// Description: The TGUIMarkerView class implements the GUI specific details
//              of the TMarkerView class.
//
// Changes:
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////

#ifdef __BORLANDC__
#include "PCHIncludes.h"
#pragma hdrstop
#endif // __BORLANDC__

#include "OSIncludes.h"

#ifndef VCL
# error This is the VCL implementation of TGUIMarkerView.
#endif

#include "GUIMarkerView.h"
#include "Utils/Util.h"

#include <cassert>

TGUIMarkerView::TGUIMarkerView()
: TGUIView( markerViewZ ),
  type( 0 ),
  visible( false ),
  color( TGUIView::GetElementColor( visualMarker ).cl ),
  lineWidth( TGUIView::GetElementWidth( visualMarker ) )
{
}

TGUIMarkerView::~TGUIMarkerView()
{
}

void
TGUIMarkerView::Paint()
{
  if( visible )
  {
    switch( type )
    {
      case 0: // No marker
        break;
      case 1: // Zero Bar
        {
          TCanvas* canvas = TGUIView::GetCanvas();
          canvas->Brush->Color = color;
          int centerY = viewTRect.Top +
                           ROUND( ( viewTRect.Height() - lineWidth ) / 2.0 );
          canvas->FillRect( TRect( viewTRect.Left, centerY,
                                     viewTRect.Right, centerY + lineWidth ) );
        }
        break;
      case 2: // Fixation Cross
        {
          int crossHeight = viewTRect.Height() / 4,
              crossWidth = crossHeight,
              crossLineWidth = crossHeight / 4;
          if( crossLineWidth < lineWidth )
            crossLineWidth = lineWidth;
          TCanvas* canvas = TGUIView::GetCanvas();
          canvas->Brush->Color = color;
          int centerY = viewTRect.Top +
                           ROUND( ( viewTRect.Height() - crossLineWidth ) / 2.0 ),
              centerX = viewTRect.Left +
                           ROUND( ( viewTRect.Width() - crossLineWidth ) / 2.0 ),
              topY =    viewTRect.Top +
                           ROUND( ( viewTRect.Height() - crossHeight ) / 2.0 ),
              leftX =   viewTRect.Left +
                           ROUND( ( viewTRect.Width() - crossWidth ) / 2.0 );
          canvas->FillRect( TRect( leftX, centerY, leftX + crossWidth,
                                                  centerY + crossLineWidth ) );
          canvas->FillRect( TRect( centerX, topY, centerX + crossLineWidth,
                                                  topY + crossHeight ) );
        }
        break;
      default:
        assert( false );
      }
  }
}

void
TGUIMarkerView::SetType( int inType )
{
  type = inType;
  TGUIView::InvalidateRect( viewTRect );
}

void
TGUIMarkerView::Show()
{
    visible = true;
    TGUIView::InvalidateRect( viewTRect );
}

void
TGUIMarkerView::Hide()
{
    visible = false;
    TGUIView::InvalidateRect( viewTRect );
}

