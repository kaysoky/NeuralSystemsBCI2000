/////////////////////////////////////////////////////////////////////////////
//
// File: GUIScoreView.cpp
//
// Date: Nov 7, 2001
//
// Author: Juergen Mellinger
//
// Description: The TGUIScoreView class implements the GUI specific details
//              of the TScoreView class.
//
// Changes: Mar 17, 2003, jm:
//          Moved buffer update from Paint() into InvalidateBuffer() to
//          adapt to new window invalidation scheme.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////

#ifdef __BORLANDC__
#include "PCHIncludes.h"
#pragma hdrstop
#endif // __BORLANDC__

#include "OSIncludes.h"

#ifndef VCL
# error This is the VCL implementation of TGUIScoreView.
#endif

#include "GUIScoreView.h"
#include "Utils/Util.h"

const char      scoreFont[] = "Arial";
const TColor    scoreColor = clGreen;

TGUIScoreView::TGUIScoreView()
: TGUIView( scoreViewZ ),
  successTrials( 0 ),
  totalTrials( 0 ),
  visible( false ),
  buffer( NULL )
{
  buffer = new Graphics::TBitmap;
  buffer->Transparent = true;
  buffer->TransparentMode = tmFixed;
  buffer->TransparentColor = clWhite;
  buffer->Canvas->Pen->Color = clWhite;
  buffer->Canvas->Brush->Color = clWhite;
  buffer->Canvas->Font->Color = TGUIView::GetElementColor( scoreViewText ).cl;
  buffer->Canvas->Font->Name = fontName;
  InvalidateBuffer();
}

TGUIScoreView::~TGUIScoreView()
{
  delete buffer;
}

void
TGUIScoreView::Paint()
{
  if( visible )
    GetCanvas()->Draw( viewTRect.Left, viewTRect.Top, buffer );
}

void
TGUIScoreView::Resized()
{
  TGUIView::Resized();
  InvalidateBuffer();
}

void
TGUIScoreView::Show()
{
  visible = true;
  TGUIView::InvalidateRect( TRect( viewTRect.Left,
                                   viewTRect.Top,
                                   viewTRect.Left + buffer->Width,
                                   viewTRect.Top + buffer->Height ) );
}

void
TGUIScoreView::Hide()
{
  visible = false;
  TGUIView::InvalidateRect( TRect( viewTRect.Left,
                                   viewTRect.Top,
                                   viewTRect.Left + buffer->Width,
                                   viewTRect.Top + buffer->Height ) );
}

void
TGUIScoreView::InvalidateBuffer()
{
  TCanvas *canvas = buffer->Canvas;
  canvas->Font->Height = viewTRect.Height() / 2;

  AnsiString  successTrialsString = IntToStr( successTrials ),
              totalTrialsString = IntToStr( totalTrials );

  int successTrialsWidth = canvas->TextWidth( successTrialsString ),
      totalTrialsWidth = canvas->TextWidth( totalTrialsString ),
      maxWidth = MAX( successTrialsWidth, totalTrialsWidth );

  buffer->Height = viewTRect.Height();
  buffer->Width = maxWidth;

  canvas->FillRect( TRect( 0, 0, buffer->Width, buffer->Height ) );
  canvas->TextOut( maxWidth - successTrialsWidth, 0, successTrialsString );
  canvas->TextOut( maxWidth - totalTrialsWidth, buffer->Height / 2, totalTrialsString );

  TGUIView::InvalidateRect( TRect( viewTRect.Left,
                                   viewTRect.Top,
                                   viewTRect.Left + buffer->Width,
                                   viewTRect.Top + buffer->Height ) );
}

